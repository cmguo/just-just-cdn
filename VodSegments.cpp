//VodSegments.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/VodSegments.h"
#include "ppbox/cdn/CdnError.h"

#include <ppbox/common/HttpFetchManager.h>

#include <util/protocol/pptv/Url.h>
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
            , bwtype_(-1)
            , video_(NULL)
        {
        }

        VodSegments::~VodSegments()
        {
            if (video_) {
                delete video_;
                video_ = NULL;
            }
        }

        void VodSegments::async_open(
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

        boost::system::error_code VodSegments::get_request(
            size_t segment, 
            boost::uint64_t& beg, 
            boost::uint64_t& end, 
            std::string& url,
            boost::system::error_code & ec)
        {
            ec.clear();
            if (segment < segments_.size()) {
                framework::string::Url url_t("http://localhost/");
                url_t.host(server_host_.host());
                url_t.svc(server_host_.svc());
                url_t.path("/" + format(segment) + "/" + name_);
                url_t.param("key", get_key());
                url = url_t.to_string();
                LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] cdn url:"<<url
                    <<" from:"<<beg
                    <<" to:"<<end);
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
            ios_service().post(boost::bind(resp, ec));
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
                        boost::bind(&VodSegments::handle_jump, this ,_1, _2));
                    return;
                case StepType::jump:
                    if (segments_.size() > 0 && video_->duration == segments_[0].duration) {
                        know_seg_count_ = true;
                        open_step_ = StepType::finish;
                    } else {
                        open_step_ = StepType::drag;
                        handle_ = fetch_mgr_.async_fetch(
                            get_drag_url(),
                            dns_vod_drag_server,
                            boost::bind(&VodSegments::handle_drag, this, _1, _2));
                        return;
                    }
                    break;
                case StepType::drag:
                    open_step_ = StepType::finish;
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

            VodJumpInfoNoDrag jump_info;
            if (!ecc) {
                util::archive::XmlIArchive <> ia(buf);
                ia >> (VodJumpInfo&)jump_info;
                if (!ia) {
                    util::archive::XmlIArchive<> iar(buf);
                    iar >> jump_info;
                    if (!iar) {
                        ecc = error::bad_file_format;
                    }
                }
            }//else

            if (!ecc)
            {
                set_info_by_video(jump_info.video);
                server_host_ = jump_info.server_host;
                bwtype_ = jump_info.BWType;
                server_time_ = jump_info.server_time.to_time_t();
                local_time_ = Time::now();

                if (jump_info.block_size != 0) {
                    segments_.push_back(jump_info.firstseg);
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


            VodDragInfoNew drag_info;
            if (!ecc) {
                util::archive::XmlIArchive <> ia(buf);
                ia >> drag_info;
                if (!ia) {
                    util::archive::XmlIArchive <> iar(buf);
                    VodDragInfo drag_info_new;
                    iar >> drag_info_new;
                    if (!iar) {
                        ecc = error::bad_file_format;
                    } else {
                        drag_info = drag_info_new;
                    }
                }
            }

            if (!ecc)
            {
                set_info_by_video(drag_info.video);
                know_seg_count_ = true;
                std::vector<VodSegmentNew> & segmentsTmp = drag_info.segments;
                size_t segment_size = segments_.size();
                for (size_t i = 0;  i < segmentsTmp.size(); ++i) {
                    if (i == 0 && segments_.size() > 0)
                        continue;
                    if (i < segment_size) {
                        boost::uint64_t filesize = segments_[i].file_length;
                        segments_[i] = segmentsTmp[i];
                        assert(filesize == segmentsTmp[i].file_length);
                    } else {
                        segments_.push_back(segmentsTmp[i]);
                    }
                }
            }

            fetch_mgr_.close(handle_);
            handle_async_open(ecc);
        }


        void VodSegments::set_info_by_video(
            VodVideo & video)
        {
            if (NULL == video_) {
                video_ = new VodVideo(video);
            } else {
                *video_ = video;
            }
        }

        std::string VodSegments::get_key() const
        {
            return util::protocol::pptv::gen_key_from_time(server_time_ + (Time::now() - local_time_).total_seconds());
        }


        size_t VodSegments::segment_count()
        {
            size_t ret = size_t(-1);
            if (know_seg_count_) {
                ret = segments_.size();
            }
            return ret;
        }

        boost::uint64_t VodSegments::segment_head_size(
            size_t segment)
        {
            boost::uint64_t ret = boost::uint64_t(-1);
            if (segments_.size() > segment) {
                return segments_[segment].head_length;
            }
            return ret;
        }

        boost::uint64_t VodSegments::segment_body_size(
            size_t segment)
        {
            boost::uint64_t ret = boost::uint64_t(-1);
            if (segments_.size() > segment) {
                return (segments_[segment].file_length - segments_[segment].head_length);
            }
            return ret;
        }

        boost::uint64_t VodSegments::segment_size(
            size_t segment)
        {
            boost::uint64_t ret = boost::uint64_t(-1);
            if (segments_.size() > segment ) {
                ret = segments_[segment].file_length;
            }
            return ret;
        }

        boost::uint32_t VodSegments::segment_time(size_t segment)
        {
            boost::uint32_t ret = boost::uint32_t(-1);
            if (segments_.size() > segment ) {
                ret = segments_[segment].duration;
            }
            return ret;
        }

        boost::system::error_code VodSegments::get_duration(
            DurationInfo & info,
            boost::system::error_code & ec)
        {
            ec.clear();
            if (NULL != video_) {
                info.total = video_->duration;
                info.begin = 0;
                info.end = video_->duration;
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
                segments_.push_back(newSegment);
            } else if (segment_count() > segment) {
            } else {
                assert(false);
            }
        }

        void VodSegments::update_segment_duration(size_t segment, boost::uint32_t time)
        {
            if (segments_.size() > segment ) {
                segments_[segment].duration = time;
            }
        }

        void VodSegments::update_segment_file_size(size_t segment, boost::uint64_t fsize)
        {
            if (segments_.size() > segment) {
                if (!know_seg_count_ 
                    && segments_[segment].file_length == boost::uint64_t(-1)) {
                        segments_[segment].file_length = fsize;
                }
            }

            segment++;
            if (!know_seg_count_ && segments_.size() == segment) {
                // add a segment
                VodSegmentNew vod_seg;
                vod_seg.duration = boost::uint32_t(-1);
                vod_seg.file_length = boost::uint64_t(-1);
                vod_seg.head_length = boost::uint64_t(-1);
                segments_.push_back(vod_seg);
            }
        }

        void VodSegments::update_segment_head_size(size_t segment, boost::uint64_t hsize)
        {
            if (segments_.size() > segment) {
                if (!know_seg_count_ 
                    && segments_[segment].head_length == boost::uint64_t(-1)) {
                        segments_[segment].head_length = hsize;
                }
            }

            segment++;
            if (!know_seg_count_ && segments_.size() == segment) {
                // add a segment
                VodSegmentNew vod_seg;
                vod_seg.duration = boost::uint32_t(-1);
                vod_seg.file_length = boost::uint64_t(-1);
                vod_seg.head_length = boost::uint64_t(-1);
                segments_.push_back(vod_seg);
            }
        }

        bool VodSegments::is_know_seg() const
        {
            return know_seg_count_;
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
                bwtype_ = framework::string::parse<boost::int32_t>(strBwtype);
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