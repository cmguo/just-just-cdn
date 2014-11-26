// PptvLive2.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/pptv/PptvLive2.h"

#include <ppbox/common/DomainName.h>

#include <framework/string/Format.h>
using namespace framework::string;
#include <framework/logger/StreamRecord.h>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.cdn.PptvLive2", framework::logger::Debug);

#ifndef PPBOX_DNS_LIVE2_JUMP
#  define PPBOX_DNS_LIVE2_JUMP "(tcp)(v4)live.dt.synacast.com:80"
#endif

namespace ppbox
{
    namespace cdn
    {

        DEFINE_DOMAIN_NAME(dns_live2_jump, PPBOX_DNS_LIVE2_JUMP);

        PptvLive2::PptvLive2(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : PptvLive(io_svc, url)
            , open_step_(StepType::closed)
        {
        }

        PptvLive2::~PptvLive2()
        {
        }

        void PptvLive2::async_open(
            response_type const & resp)
        {
            assert(StepType::closed == open_step_);

            set_response(resp);
            boost::system::error_code ec;

            if (url_.path().find('-') != std::string::npos) {
                // "[StreamID]-[Interval]-[datareate]
                std::vector<std::string> strs;
                slice<std::string>(url_.path(), std::inserter(strs, strs.end()), "-");
                if (strs.size() >= 3) {
                    jump_info_.video.name = strs[0].substr(1);
                    jump_info_.video.rid = jump_info_.video.name;
                    parse2(strs[2], jump_info_.video.bitrate);
                    parse2(strs[1], seg_.interval);
                } else {
                    ec = error::bad_url_format;
                }
            } else {
                ec = error::bad_url_format;
            }

            handle_async_open(ec);
        }

        void PptvLive2::handle_async_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                response(ec);
                return;
            }

            framework::string::Url url;

            switch(open_step_) {
                case StepType::closed:
                    if (jump_ && video_ && segment_) {
                        open_step_ = StepType::finish;
                        response(ec);
                        break;
                    }
                    open_step_ = StepType::jumping;
                    LOG_INFO("jump: start");
                    async_fetch(
                        get_jump_url(url),
                        dns_live2_jump,
                        jump_info_, 
                        boost::bind(&PptvLive2::handle_async_open, this ,_1));
                    break;
                case StepType::jumping:
                    jump_info_.video.delay = jump_info_.delay_play_time;
                    set_video(jump_info_.video);
                    set_jump(jump_info_.jump);
                    set_segment(seg_);
                    open_step_ = StepType::finish;
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
            url.path("/live2/" + jump_info_.video.rid);
            return url;
        }

    } // namespace cdn
} // namespace ppbox

// PptvLive2.cpp