// PptvVod1.cpp

#include "just/cdn/Common.h"
#include "just/cdn/CdnError.h"
#include "just/cdn/pptv/PptvVod1.h"

#include <just/common/DomainName.h>
#ifndef JUST_DISABLE_CERTIFY
#include <just/certify/Certifier.h>
#endif

#include <util/protocol/pptv/Url.h>
using namespace util::protocol;

#include <framework/string/Format.h>
using namespace framework::string;
#include <framework/logger/StreamRecord.h>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.cdn.PptvVod1", framework::logger::Debug);

#ifndef JUST_DNS_VOD_JUMP
#  define JUST_DNS_VOD_JUMP "(tcp)(v4)jump.synacast.com:80"
#endif

#ifndef JUST_DNS_VOD_DRAG
#  define JUST_DNS_VOD_DRAG "(tcp)(v4)drag.synacast.com:80"
#endif

namespace just
{
    namespace cdn
    {

        DEFINE_DOMAIN_NAME(dns_vod_jump, JUST_DNS_VOD_JUMP);
        DEFINE_DOMAIN_NAME(dns_vod_drag, JUST_DNS_VOD_DRAG);

        PptvVod1::PptvVod1(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : PptvVod(io_svc, url)
            , open_step_(StepType::closed)
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
            if (path.size() > 4 && path.substr(path.size() - 4) == ".mp4") {
                if (path.find('%') == std::string::npos) { // ÖÐÎÄÃûencode
                    path = Url::encode(path, ".");
                }
            } else {
                std::string key = url_.param_or("cdn.key", "kioe257ds");
#ifndef JUST_DISABLE_CERTIFY
                cert_.certify_url(url_, key, ec);
#endif
                if (!ec) {
                    path = pptv::url_decode(path, key);
                    StringToken st(path, "||");
                    boost::system::error_code ec;
                    if (!st.next_token(ec)) {
                        path = st.remain();
                    } else {
                        ec = error::bad_url_format;
                    }
                }
            }

            url_.path("/" + path);
            drag_info_.video.rid = path;

            handle_async_open(ec);
        }

        void PptvVod1::async_open2()
        {
            boost::system::error_code ec;
            handle_async_open(ec);
        }

        void PptvVod1::handle_async_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                response(ec);
                return;
            }

            framework::string::Url url;

            switch(open_step_) {
                case StepType::closed:
                    open_step_ = StepType::jumping;
                    if (!jump_) {
                        async_fetch(
                            get_jump_url(url),
                            dns_vod_jump,
                            jump_info_, 
                            boost::bind(&PptvVod1::handle_async_open, this, _1));
                    } else {
                        handle_async_open(ec);
                    }
                    break;
                case StepType::jumping:
                    if (jump_info_.video.is_initialized()) {
                        jump_info_.video->rid = drag_info_.video.rid;
                        set_video(jump_info_.video.get());
                    }
                    set_jump(jump_info_);
                    set_user_host(jump_info_.user_host);
                    //if (owner_type() == ot_demuxer) {
                    //    open_step_ = StepType::wait2;
                    //    response(ec);
                    //    break;;
                    //}
                case StepType::wait2:
                    open_step_ = StepType::draging;
                    async_fetch(
                        get_drag_url(url),
                        dns_vod_drag,
                        drag_info_, 
                        boost::bind(&PptvVod1::handle_async_open, this, _1));
                    break;
                case StepType::draging:
                    set_video(drag_info_.video);
                    set_segments(drag_info_.segments);
                    open_step_ = StepType::finish;
                    response(ec);
                    break;
                default:
                    //assert(0);
                    break;
            }
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

    } // cdn
} // just
