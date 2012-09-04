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
#include <framework/logger/StreamRecord.h>
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
//             assert(StepType::not_open == open_step_);

            open_logs_.resize(2);//设置日志信息个数
            tc_.start();//开始记录打开时间

            boost::system::error_code ec;
            if (jdp_url_.path().empty())
            {
                ec = error::bad_url;
            }

            set_response(resp);
            mode_ = mode;
            handle_async_open(ec);
        }

        framework::string::Url VodSegments::get_jump_url()
        {
            framework::string::Url url = jdp_url_;
            url.host(dns_vod_jump_server.host());
            url.svc(dns_vod_jump_server.svc());
            std::string name = url.path();
            url.path(std::string("/") + name + "dt");

            return url;
        }

        framework::string::Url VodSegments::get_drag_url()
        {
            framework::string::Url url = jdp_url_;
            url.host(dns_vod_drag_server.host());
            url.svc(dns_vod_drag_server.svc());
            std::string name = url.path();
            url.path(std::string("/") + name + "0drag");

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
                url.path("/" + format(segment) + std::string("/") + url.path());
                url.param("key", get_key());
                LOG_DEBUG("[get_request] cdn url:"<< url.to_string());

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
                    LOG_WARN("parse url:failure");
                }
                if (StepType::jump == open_step_) {
                    LOG_WARN("jump : failure");
                    open_logs_end(0, ec);
                    LOG_DEBUG("jump failure (" << open_logs_[0].total_elapse << " milliseconds)");
                }
                if (StepType::drag == open_step_) {
                    LOG_WARN("drag : failure");
                    open_logs_end(1, ec);
                    LOG_DEBUG("drag failure (" << open_logs_[2].total_elapse << " milliseconds)");
                }
                response(ec);
                last_error_ = ec;
                open_total_time_ = tc_.elapsed();
                return;
            }

            switch(open_step_) {
                case StepType::not_open:
                    open_step_ = StepType::jump;
                    async_fetch(
                        get_jump_url(),
                        dns_vod_jump_server,
                        jump_info_, 
                        boost::bind(&VodSegments::handle_async_open, this, _1));
                    return;

                case StepType::jump:
                    open_step_ = StepType::drag;
                    async_fetch(
                        get_drag_url(),
                        dns_vod_drag_server,
                        drag_info_, 
                        boost::bind(&VodSegments::handle_async_open, this, _1));
                    return;

                case StepType::drag:
                    break;

                default:
                    assert(0);
                    return;
            }

            response(ec);
            last_error_ = ec;
            open_total_time_ = tc_.elapsed();

            return;
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
            ppbox::data::SegmentInfo & info) const
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
            ppbox::data::DurationInfo & info,
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