// PptvLive1.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/PptvLive1.h"

#include <ppbox/common/DomainName.h>

#include <util/protocol/pptv/Base64.h>
using namespace util::protocol;

#include <framework/string/Format.h>
using namespace framework::string;
#include <framework/logger/StreamRecord.h>

#ifndef PPBOX_DNS_LIVE1_JUMP
#  define PPBOX_DNS_LIVE1_JUMP "(tcp)(v4)live.dt.synacast.com:80"
#endif

namespace ppbox
{
    namespace cdn
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.cdn.PptvLive1", framework::logger::Debug);

        DEFINE_DOMAIN_NAME(dns_live1_jump, PPBOX_DNS_LIVE1_JUMP);

        PptvLive1::PptvLive1(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : PptvLive(io_svc, url)
            , open_step_(StepType::closed)
        {
            ppbox::data::MediaBasicInfo info;
            boost::system::error_code ec;
            get_basic_info(info, ec);
            info.format = "asf";
            set_basic_info(info);

            jump_info_.seg.interval = boost::uint16_t(-1);
        }

        PptvLive1::~PptvLive1()
        {
        }

        void PptvLive1::async_open(
            response_type const & resp)
        {
            assert(StepType::closed == open_step_);

            set_response(resp);
            boost::system::error_code ec;

            std::string key = url_.param_or("cdn.key", "pplive");
            url_str_ = url_.path().substr(1);
            url_str_ = pptv::base64_decode(url_str_, key);
            if (!url_str_.empty()) {
                map_find(url_str_, "name", jump_info_.video.name, "&");
                map_find(url_str_, "channel", jump_info_.video.rid, "&");
                set_video(jump_info_.video);
            } else {
                ec = error::bad_url_format;
            }

            handle_async_open(ec);
        }

        bool PptvLive1::segment_url(
            size_t segment, 
            framework::string::Url & url,
            boost::system::error_code & ec) const
        {
            ec.clear();
            url = url_; //这里使用原始传入的播放url
            url.host(jump_->server_host.host());
            url.svc(jump_->server_host.svc());
            url.path("/live/" + video_->rid);
            url.param("url", url_str_);
            if (!jump_info_.server_hosts.empty()) {
                url.param("s", jump_info_.server_hosts.front());
            }
            url.param("st", jump_info_.proto_type);
            url.param("sl", format(jump_info_.buffer_size));
            LOG_DEBUG("[segment_url] url:" << url.to_string());
            return true;
        }

        void PptvLive1::handle_async_open(
            boost::system::error_code ec)
        {
            if (ec) {
                if (open_step_ == StepType::jumping) {
                    set_jump(jump_info_.jump);
                    set_segment(jump_info_.seg);
                    ec.clear();
                }
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
                        dns_live1_jump,
                        jump_info_, 
                        boost::bind(&PptvLive1::handle_async_open, this ,_1));
                    break;
                case StepType::jumping:
                    set_jump(jump_info_.jump);
                    set_segment(jump_info_.seg);
                    open_step_ = StepType::finish;
                    response(ec);
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        framework::string::Url&  PptvLive1::get_jump_url(
            framework::string::Url & url) const
        {
            url = url_;
            url.host(dns_live1_jump.host());
            url.svc(dns_live1_jump.svc());
            url.path("/live1/" + jump_info_.video.rid);
            return url;
        }

    } // namespace cdn
} // namespace ppbox
