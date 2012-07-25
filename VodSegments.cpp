//VodSegments.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/VodSegments.h"
#include "ppbox/cdn/CdnError.h"

#include <ppbox/common/HttpFetchManager.h>

#include <util/protocol/pptv/TimeKey.h>
#include <util/archive/XmlIArchive.h>
#include <util/archive/ArchiveBuffer.h>

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("VodSegments", 0);

#ifndef PPBOX_DNS_VOD_JUMP
#define PPBOX_DNS_VOD_JUMP "(tcp)(v4)jump.150hi.com:80"
#endif

#ifndef PPBOX_DNS_VOD_DRAG
#define PPBOX_DNS_VOD_DRAG "(tcp)(v4)drag.150hi.com:80"
#endif

namespace ppbox
{
    namespace cdn
    {

        static const framework::network::NetName dns_vod_jump_server(PPBOX_DNS_VOD_JUMP);
        static const framework::network::NetName dns_vod_drag_server(PPBOX_DNS_VOD_DRAG);

        VodSegments::VodSegments(
            boost::asio::io_service & io_svc)
            : SegmentBase(io_svc)
            , fetch_mgr_(util::daemon::use_module<ppbox::common::HttpFetchManager>(global_daemon()))
            , handle_(NULL)
            , open_step_(StepType::not_open)
            , know_seg_count_(false)
        {
        }

        VodSegments::~VodSegments()
        {
        }

        void VodSegments::async_open(
            OpenMode mode,
            response_type const & resp)
        {
            assert(StepType::not_open == open_step_);

            boost::system::error_code ec;
            if (name_.empty())
            {
                ec = error::bad_url;
            }
            resp_ = resp;

            handle_async_open(ec);
        }

        void VodSegments::cancel(boost::system::error_code & ec)
        {
            fetch_mgr_.cancel(handle_);
        }

        void VodSegments::close(boost::system::error_code & ec)
        {
            fetch_mgr_.close(handle_);
        }

        bool VodSegments::is_open()
        {
            switch (open_step_) {
                case StepType::drag:
                case StepType::finish:
                    return true;
                default:
                    return false;
            }
        }

        framework::string::Url VodSegments::get_jump_url()
        {
            framework::string::Url url("http://loaclhost/");
            url.host(dns_vod_jump_server.host());
            url.svc(dns_vod_jump_server.host_svc());
            url.path("/" + name_ + "dt");

            return url;
        }

        framework::string::Url VodSegments::get_drag_url()
        {
            framework::string::Url url("http://loaclhost/");
            url.host(dns_vod_drag_server.host());
            url.svc(dns_vod_drag_server.host_svc());
            url.path("/" + name_ + "0drag");
            url.param("type", "ppbox");

            return url;
        }

        boost::system::error_code VodSegments::segment_url(
            size_t segment, 
            framework::string::Url & url,
            boost::system::error_code & ec)
        {
            ec.clear();
            if (segment < drag_info_.segments.size()) {
                url.protocol("http");
                url.host(jump_info_.server_host.host());
                url.svc(jump_info_.server_host.svc());
                url.path("/" + format(segment) + "/" + name_);
                url.param("key", get_key());

                LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] cdn url:"<<url.to_string());

                url.param("rid", drag_info_.segments[segment].va_rid);
                url.param("blocksize", format(drag_info_.segments[segment].block_size));
                url.param("filelength", format(drag_info_.segments[segment].file_length));
                url.param("headlength", format(drag_info_.segments[segment].head_length));
            } else {
                ec = error::item_not_exist;
            }
            return ec;
        }

        void VodSegments::response(
            boost::system::error_code const & ec)
        {
            SegmentBase::response_type resp;
            resp.swap(resp_);
            resp(ec);
        }

        void VodSegments::handle_async_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                if (StepType::not_open == open_step_) {
                    LOG_S(Logger::kLevelAlarm, "parse url:failure");
                }
                if (StepType::jump == open_step_) {
                    LOG_S(Logger::kLevelAlarm, "jump : failure");
                }
                if (StepType::drag == open_step_) {
                    LOG_S(Logger::kLevelAlarm, "drag : failure");
                }
                response(ec);
                return;
            }

            switch(open_step_) {
                case StepType::not_open:
                    open_step_ = StepType::jump;
                    handle_ = fetch_mgr_.async_fetch(
                        get_jump_url(),
                        dns_vod_jump_server,
                        boost::bind(&VodSegments::handle_jump, this, _1, _2));
                    return;
                case StepType::jump:
                    if (jump_info_.firstseg.is_initialized() && jump_info_.video->duration == jump_info_.firstseg->duration) {
                        open_step_ = StepType::finish;
                    } else {
                        open_step_ = StepType::drag;
                        handle_ = fetch_mgr_.async_fetch(
                            get_drag_url(),
                            dns_vod_drag_server,
                            boost::bind(&VodSegments::handle_drag, this, _1, _2));
                    }
                    break;
                default:
                    assert(0);
                    return;
            }

            response(ec);
            return;
        }

        void VodSegments::handle_jump(
            boost::system::error_code const & ec, 
            boost::asio::streambuf &buf)
        {
            boost::system::error_code  ecc = ec;
            std::string buffer = boost::asio::buffer_cast<char const *>(buf.data());
            LOG_S(Logger::kLevelDebug2, "[handle_jump] jump buffer: " << buffer);

            if (!ecc) {
                util::archive::XmlIArchive <> ia(buf);
                ia >> jump_info_;
                if (!ia) {
                    ecc = error::bad_file_format;
                } else {
                    server_time_ = jump_info_.server_time.to_time_t();
                    local_time_ = Time::now();

                    if (jump_info_.video.is_initialized()) {
                        drag_info_.video.name = jump_info_.video->name;
                        drag_info_.video.type = jump_info_.video->type;
                        drag_info_.video.bitrate = jump_info_.video->bitrate;
                        drag_info_.video.filesize = jump_info_.video->filesize;
                        drag_info_.video.duration = jump_info_.video->duration;
                        drag_info_.video.width = jump_info_.video->width;
                        drag_info_.video.height = jump_info_.video->height;
                    }
                    if (jump_info_.firstseg.is_initialized()) {
                        VodSegment seg;
                        seg.head_length = jump_info_.firstseg->head_length;
                        seg.file_length = jump_info_.firstseg->file_length;
                        seg.duration = jump_info_.firstseg->duration;
                        seg.va_rid = jump_info_.firstseg->va_rid;
                        seg.duration_offset = jump_info_.firstseg->duration_offset;
                        seg.duration_offset_us = jump_info_.firstseg->duration_offset_us;
                        seg.block_size = jump_info_.firstseg->block_size;
                        seg.block_num = jump_info_.firstseg->block_num;

                        drag_info_.segments.push_back(seg);
                        know_seg_count_ = true;
                    }

                }
              
            }

            fetch_mgr_.close(handle_);
            handle_async_open(ecc);
        }

        void VodSegments::handle_drag(
            boost::system::error_code const & ec, 
            boost::asio::streambuf &buf)
        {
            boost::system::error_code  ecc = ec;
            std::string buffer = boost::asio::buffer_cast<char const *>(buf.data());
            LOG_S(Logger::kLevelDebug2, "[handle_drag] drag buffer: " << buffer);
            if (!ecc) {
                util::archive::XmlIArchive <> ia(buf);
                ia >> drag_info_;
                if (!ia) {
                    ecc = error::bad_file_format;
                } else {
                    know_seg_count_ = true;
                }
            }

            fetch_mgr_.close(handle_);
            handle_async_open(ecc);
        }


        std::string VodSegments::get_key() const
        {
            return util::protocol::pptv::gen_key_from_time(server_time_ + (Time::now() - local_time_).total_seconds());
        }


        size_t VodSegments::segment_count()
        {
            size_t ret = size_t(-1);
            ret = drag_info_.segments.size();
            return ret;
        }

        void VodSegments::segment_info(
            size_t segment, 
            common::SegmentInfo & info)
        {
            if (drag_info_.segments.size() > segment) {
                info.head_size = drag_info_.segments[segment].head_length;
                info.size = drag_info_.segments[segment].file_length;
                info.time = drag_info_.segments[segment].duration;
            } else {
                info.head_size = boost::uint64_t(-1);
                info.size = boost::uint64_t(-1);
                info.time = boost::uint64_t(-1);
            }
        } 

        boost::system::error_code VodSegments::get_duration(
            common::DurationInfo & info,
            boost::system::error_code & ec)
        {
            ec.clear();
            if (0 != drag_info_.video.duration) {
                info.total = drag_info_.video.duration;
                info.begin = 0;
                info.end = drag_info_.video.duration;
                info.redundancy = 0;
                info.interval = 0;
            } else {
                ec = error::not_open;
            }
            return ec;
        }

        void VodSegments::update_segment(size_t segment)
        {
            if (segment_count() == segment ) {
                VodSegmentNew newSegment;
                newSegment.duration = boost::uint32_t(-1);
                newSegment.file_length = boost::uint64_t(-1);
                newSegment.head_length = boost::uint64_t(-1);
                drag_info_.segments.push_back(newSegment);
            } else if (segment_count() > segment) {
            } else {
                assert(false);
            }
        }

        void VodSegments::update_segment_duration(size_t segment, boost::uint32_t time)
        {
            if (drag_info_.segments.size() > segment ) {
                drag_info_.segments[segment].duration = time;
            }
        }

        void VodSegments::update_segment_file_size(size_t segment, boost::uint64_t fsize)
        {
            if (drag_info_.segments.size() > segment) {
                if (!know_seg_count_ && drag_info_.segments[segment].file_length == boost::uint64_t(-1)) {
                        drag_info_.segments[segment].file_length = fsize;
                }
            }

            segment++;
            if (!know_seg_count_ && drag_info_.segments.size() == segment) {
                // add a segment
                VodSegmentNew vod_seg;
                vod_seg.duration = boost::uint32_t(-1);
                vod_seg.file_length = boost::uint64_t(-1);
                vod_seg.head_length = boost::uint64_t(-1);
                drag_info_.segments.push_back(vod_seg);
            }
        }

        void VodSegments::update_segment_head_size(size_t segment, boost::uint64_t hsize)
        {
            if (drag_info_.segments.size() > segment) {
                if (!know_seg_count_ 
                    && drag_info_.segments[segment].head_length == boost::uint64_t(-1)) {
                        drag_info_.segments[segment].head_length = hsize;
                }
            }

            segment++;
            if (!know_seg_count_ && drag_info_.segments.size() == segment) {
                // add a segment
                VodSegmentNew vod_seg;
                vod_seg.duration = boost::uint32_t(-1);
                vod_seg.file_length = boost::uint64_t(-1);
                vod_seg.head_length = boost::uint64_t(-1);
                drag_info_.segments.push_back(vod_seg);
            }
        }

        void VodSegments::set_url(std::string const &url)
        {
            SegmentBase::set_url(url);
            boost::system::error_code ec;
            std::string::size_type slash = url.find('|');
            if (slash == std::string::npos) {
                return;
            } 
            std::string key = url.substr(0, slash);
            std::string playlink = url.substr(slash + 1);

            playlink = framework::string::Url::decode(playlink);
            framework::string::Url request_url(playlink);
            playlink = request_url.path().substr(1);

            std::string strBwtype = request_url.param("bwtype");
            if(!strBwtype.empty()) {
                jump_info_.BWType = framework::string::parse<boost::int32_t>(strBwtype);
            }

            if (playlink.size() > 4 && playlink.substr(playlink.size() - 4) == ".mp4") {
                if (playlink.find('%') == std::string::npos) {
                    playlink = Url::encode(playlink, ".");
                }
            } else {
                playlink = util::protocol::pptv::url_decode(playlink, key);
                StringToken st(playlink, "||");
                if (!st.next_token(ec)) {
                    playlink = st.remain();
                }
            }
            name_ = playlink;
        }

        boost::system::error_code VodSegments::reset(size_t& segment)
        {
            segment = 0;
            return boost::system::error_code();
        }

    }//cdn
}//ppbox

//VodSegments.cpp