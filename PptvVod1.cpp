// PptvVod1.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/PptvVod1.h"

#include <ppbox/common/DomainName.h>
#include <ppbox/certify/Certifier.h>

#include <util/protocol/pptv/Url.h>
using namespace util::protocol;

#include <framework/string/Format.h>
using namespace framework::string;
#include <framework/logger/StreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PptvVod1", 0);

#ifndef PPBOX_DNS_VOD_JUMP
#  define PPBOX_DNS_VOD_JUMP "(tcp)(v4)jump.150hi.com:80"
#endif

#ifndef PPBOX_DNS_VOD_DRAG
#  define PPBOX_DNS_VOD_DRAG "(tcp)(v4)drag.150hi.com:80"
#endif

namespace ppbox
{
    namespace cdn
    {

        DEFINE_DOMAIN_NAME(dns_vod_jump, PPBOX_DNS_VOD_JUMP);
        DEFINE_DOMAIN_NAME(dns_vod_drag, PPBOX_DNS_VOD_DRAG);

        PptvVod1::PptvVod1(
            boost::asio::io_service & io_svc)
            : PptvVod(io_svc)
            , open_step_(StepType::not_open)
            , know_seg_count_(false)
        {
        }

        PptvVod1::~PptvVod1()
        {
        }

        void PptvVod1::async_open(
            response_type const & resp)
        {
            set_response(resp);

            boost::system::error_code ec;
            std::string path = url_.path().substr(1);
            std::string key = "kioe257ds";
            if (path.size() > 4 && path.substr(path.size() - 4) == ".mp4") {
                if (path.find('%') == std::string::npos) { // ÖÐÎÄÃûencode
                    path = Url::encode(path, ".");
                }
            } else {
                std::string key;
                cert_.certify_url(url_, key, ec);
                if (!ec) {
                    path = pptv::url_decode(path, key);
                    StringToken st(path, "||");
                    boost::system::error_code ec;
                    if (!st.next_token(ec)) {
                        path = st.remain();
                    } else {
                        ec = error::bad_url;
                    }
                }
            }

            url_.path("/" + path);
            handle_async_open(ec);
        }

        framework::string::Url & PptvVod1::get_jump_url(
            framework::string::Url & url)
        {
            url = url_;
            url.host(dns_vod_jump.host());
            url.svc(dns_vod_jump.svc());
            url.path(url.path() + "dt");

            return url;
        }

        framework::string::Url & PptvVod1::get_drag_url(
            framework::string::Url & url)
        {
            url = url_;
            url.host(dns_vod_drag.host());
            url.svc(dns_vod_drag.svc());
            std::string name = url.path();
            url.path(url.path() + "0drag");

            return url;
        }

        void PptvVod1::handle_async_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                if (StepType::jump == open_step_) {
                    LOG_WARN("jump : failure");
                    LOG_DEBUG("jump failure (" << open_logs_[0].total_elapse << " milliseconds)");
                }
                if (StepType::drag == open_step_) {
                    LOG_WARN("drag : failure");
                    LOG_DEBUG("drag failure (" << open_logs_[2].total_elapse << " milliseconds)");
                }
                response(ec);
                return;
            }

            framework::string::Url url;

            switch(open_step_) {
                case StepType::not_open:
                    open_step_ = StepType::jump;
                    async_fetch(
                        get_jump_url(url),
                        dns_vod_jump,
                        jump_info_, 
                        boost::bind(&PptvVod1::handle_async_open, this, _1));
                    break;
                case StepType::jump:
                    set_user_host(jump_info_.user_host);
                    set_jump(jump_info_);
                    if (jump_info_.video.is_initialized())
                        set_video(jump_info_.video.get());
                    open_step_ = StepType::drag;
                    async_fetch(
                        get_drag_url(url),
                        dns_vod_drag,
                        drag_info_, 
                        boost::bind(&PptvVod1::handle_async_open, this, _1));
                    break;
                case StepType::drag:
                    set_video(drag_info_.video);
                    set_segments(drag_info_.segments);
                    response(ec);
                    break;
                default:
                    assert(0);
                    break;
            }
        }

    } // cdn
} // ppbox
