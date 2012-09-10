// PptvLive3.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/PptvLive3.h"

#include <ppbox/common/DomainName.h>

#include <framework/string/Format.h>
using namespace framework::string;
#include <framework/logger/StreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PptvLive3", 0);

#ifndef PPBOX_DNS_LIVE2_PLAY
#  define PPBOX_DNS_LIVE2_PLAY "(tcp)(v4)epg.api.pptv.com:80"
#endif

namespace ppbox
{
    namespace cdn
    {

        DEFINE_DOMAIN_NAME(dns_live2_play, PPBOX_DNS_LIVE2_PLAY);

        PptvLive3::PptvLive3(
            boost::asio::io_service & io_svc)
            : PptvLive(io_svc)
            , open_step_(StepType::not_open)
        {
        }

        PptvLive3::~PptvLive3()
        {
        }

        void PptvLive3::async_open(
            response_type const & resp)
        {
            assert(StepType::not_open == open_step_);

            set_response(resp);
            boost::system::error_code ec;
            handle_async_open(ec);
        }

        void PptvLive3::handle_async_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                if (StepType::not_open == open_step_) {
                    LOG_WARN("parse url:failure");
                }
                if (ec != boost::asio::error::would_block) {
                    if (StepType::playing == open_step_) {
                        LOG_WARN("play : failure"); 
                        LOG_DEBUG("play failure (" << open_logs_[0].total_elapse << " milliseconds)");
                    }
                }
                response(ec);
                return;
            }

            switch(open_step_) {
                case StepType::not_open:
                    open_step_ = StepType::playing;
                    LOG_INFO("jump: start");
                    {
                        framework::string::Url url;
                        async_fetch(
                            get_play_url(url),
                            dns_live2_play,
                            play_info_, 
                            boost::bind(&PptvLive3::handle_async_open, this ,_1));
                    }
                    break;
                case StepType::playing:
                    {
                        set_user_host(play_info_.uh);
                        Video & video = play_info_.channel.stream[0];
                        video.duration = play_info_.channel.jump;
                        video.delay = play_info_.channel.delay;
                        set_video(video);
                        set_jump(play_info_.jump);
                        set_segment(play_info_.channel.seg);
                    }
                    response(ec);
                    break;
                default:
                    assert(0);
                    break;
            }
        }

        framework::string::Url&  PptvLive3::get_play_url(
            framework::string::Url & url) const
        {
            url = url_;
            url.host(dns_live2_play.host());
            url.svc(dns_live2_play.svc());
            url.path("/live2/" + video_->rid);
            return url;
        }

    } // namespace cdn
} // namespace ppbox

// PptvLive3.cpp