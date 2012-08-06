//VodSegments.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/VodSegments.h"

#include <util/protocol/pptv/Url.h>
#include <util/protocol/pptv/TimeKey.h>
#include <util/archive/XmlIArchive.h>
#include <util/archive/ArchiveBuffer.h>

#include <framework/string/StringToken.h>
#include <framework/string/Url.h>
#include <framework/string/Format.h>
using namespace framework::string;
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

        PPBOX_REGISTER_SEGMENT(ppvod, VodSegments);

        VodSegments::VodSegments(
            boost::asio::io_service & io_svc)
            : PptvSegments(io_svc)
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

            open_logs_.resize(2);

            boost::system::error_code ec;
            if (jdp_url_.path().empty())
            {
                ec = error::bad_url;
            }

            PptvSegments::set_response(resp);

            handle_async_open(ec);
        }

        framework::string::Url VodSegments::get_jump_url()
        {
            framework::string::Url url = jdp_url_;
            url.host(dns_vod_jump_server.host());
            url.svc(dns_vod_jump_server.host_svc());
            std::string name = url.path();
            url.path(name + "dt");

            return url;
        }

        framework::string::Url VodSegments::get_drag_url()
        {
            framework::string::Url url = jdp_url_;
            url.host(dns_vod_drag_server.host());
            url.svc(dns_vod_drag_server.host_svc());
            std::string name = url.path();
            url.path( name + "0drag");

            return url;
        }

        boost::system::error_code VodSegments::segment_url(
            size_t segment, 
            framework::string::Url & url,
            boost::system::error_code & ec)
        {
            ec.clear();
            if (segment < drag_info_.segments.size()) {
                url = cdn_url_;
                url.host(jump_info_.server_host.host());
                url.svc(jump_info_.server_host.svc());
                url.path("/" + format(segment) + url.path());
                url.param("key", get_key());
                LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] cdn url:"<<url.to_string());

                framework::string::Url cdn_jump_param(url_.param("cdn.jump"));
                if (cdn_jump_param.param("bwtype").empty()) {
                    cdn_jump_param.param("bwtype", format(jump_info_.BWType));
                    url.param("cdn.jump", cdn_jump_param.to_string());
                }

                framework::string::Url cdn_drag_param("http://localhost/");
                cdn_drag_param.param("rid", drag_info_.segments[segment].va_rid);
                cdn_drag_param.param("blocksize", format(drag_info_.segments[segment].block_size));
                cdn_drag_param.param("filelength", format(drag_info_.segments[segment].file_length));
                cdn_drag_param.param("headlength", format(drag_info_.segments[segment].head_length));
                url.param("cdn.drag.segment", cdn_drag_param.to_string());

            } else {
                ec = error::item_not_exist;
            }
            return ec;
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
                    PptvSegments::open_logs_end(get_fetch().http_stat(), 0, ec);
                    LOG_S(Logger::kLevelDebug, "jump failure (" << open_logs_[0].total_elapse << " milliseconds)");
                }
                if (StepType::drag == open_step_) {
                    LOG_S(Logger::kLevelAlarm, "drag : failure");
                    PptvSegments::open_logs_end(get_fetch().http_stat(), 1, ec);
                    LOG_S(Logger::kLevelDebug, "drag failure (" << open_logs_[2].total_elapse << " milliseconds)");
                }
                PptvSegments::response(ec);
                PptvSegments::last_error_ = ec;
                return;
            }

            switch(open_step_) {
                case StepType::not_open:
                    open_step_ = StepType::jump;
                    get_fetch().async_fetch(
                        get_jump_url(),
                        dns_vod_jump_server,
                        boost::bind(&VodSegments::handle_jump, this, _1, _2));
                    return;
                case StepType::jump:
                    if (jump_info_.firstseg.is_initialized() && jump_info_.video->duration == jump_info_.firstseg->duration) {
                        open_step_ = StepType::finish;
                    } else {
                        open_step_ = StepType::drag;
                        get_fetch().async_fetch(
                            get_drag_url(),
                            dns_vod_drag_server,
                            boost::bind(&VodSegments::handle_drag, this, _1, _2));
                    }
                    break;
                default:
                    assert(0);
                    return;
            }

            PptvSegments::response(ec);
            PptvSegments::last_error_ = ec;

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
                    local_time_ = Time::now();

                    if (jump_info_.video.is_initialized()) {
                        drag_info_.video = jump_info_.video.get();
                    }
                    if (jump_info_.firstseg.is_initialized()) {
                        drag_info_.segments.push_back(jump_info_.firstseg.get());
                        know_seg_count_ = true;
                    }
                }
              
            }
            PptvSegments::server_host_ = jump_info_.server_host.to_string();
            PptvSegments::open_logs_end(get_fetch().http_stat(), 0, ecc);
            get_fetch().close();
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
            PptvSegments::open_logs_end(get_fetch().http_stat(), 1, ecc);

            get_fetch().close();
            handle_async_open(ecc);
        }


        std::string VodSegments::get_key() const
        {
            return util::protocol::pptv::gen_key_from_time(
                jump_info_.server_time.to_time_t() + (Time::now() - local_time_).total_seconds());
        }


        size_t VodSegments::segment_count() const
        {
            size_t ret = size_t(-1);
            ret = drag_info_.segments.size();
            return ret;
        }

        void VodSegments::segment_info(
            size_t segment,
            common::SegmentInfo & info) const
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

        void VodSegments::set_url(
            framework::string::Url const & url)
        {
            PptvSegments::set_url(url);
        }


    }//cdn
}//ppbox

//VodSegments.cpp