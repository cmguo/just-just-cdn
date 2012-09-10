// PptvLive2.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/PptvLive2.h"

#include <ppbox/common/DomainName.h>

#include <framework/string/Format.h>
using namespace framework::string;
#include <framework/logger/StreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PptvLive2", 0);

#ifndef PPBOX_DNS_LIVE2_JUMP
#  define PPBOX_DNS_LIVE2_JUMP "(tcp)(v4)live.dt.synacast.com:80"
#endif

namespace ppbox
{
    namespace cdn
    {

        DEFINE_DOMAIN_NAME(dns_live2_jump, PPBOX_DNS_LIVE2_JUMP);

        PptvLive2::PptvLive2(
            boost::asio::io_service & io_svc)
            : PptvLive(io_svc)
            , open_step_(StepType::not_open)
        {
        }

        PptvLive2::~PptvLive2()
        {
        }

        void PptvLive2::async_open(
            response_type const & resp)
        {
            assert(StepType::not_open == open_step_);

            set_response(resp);
            boost::system::error_code ec;

            if (url_.path().find('-') != std::string::npos) {
                // "[StreamID]-[Interval]-[datareate]
                std::vector<std::string> strs;
                slice<std::string>(url_.path(), std::inserter(strs, strs.end()), "-");
                if (strs.size() >= 3) {
                    jump_info_.video.name = strs[0];
                    jump_info_.video.rid = strs[0];
                    parse2(strs[2], jump_info_.video.bitrate);
                    set_video(jump_info_.video);
                    parse2(strs[1], seg_.interval);
                } else {
                    ec = error::bad_url;
                }
            } else {
                ec = error::bad_url;
            }

            handle_async_open(ec);
        }

        void PptvLive2::handle_async_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                if (StepType::not_open == open_step_) {
                    LOG_WARN("parse url:failure");
                }
                if (ec != boost::asio::error::would_block) {
                    if (StepType::jumping == open_step_) {
                        LOG_WARN("jump : failure"); 
                        LOG_DEBUG("jump failure (" << open_logs_[0].total_elapse << " milliseconds)");
                    }
                }
                response(ec);
                return;
            }

            switch(open_step_) {
                case StepType::not_open:
                    open_step_ = StepType::jumping;
                    LOG_INFO("jump: start");
                    {
                        framework::string::Url url;
                        async_fetch(
                            get_jump_url(url),
                            dns_live2_jump,
                            jump_info_, 
                            boost::bind(&PptvLive2::handle_async_open, this ,_1));
                    }
                    break;
                case StepType::jumping:
                    {
                        jump_info_.video.delay = jump_info_.delay_play_time;
                        set_jump(jump_info_.jump);
                        set_segment(seg_);
                    }
                    response(ec);
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        framework::string::Url&  PptvLive2::get_jump_url(
            framework::string::Url & url) const
        {
            url = url_;
            url.host(dns_live2_jump.host());
            url.svc(dns_live2_jump.svc());
            url.path("/live2/" + video_->rid);
            return url;
        }

    } // namespace cdn
} // namespace ppbox

// PptvLive2.cpp