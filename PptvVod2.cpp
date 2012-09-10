//PptvVod2.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/PptvVod2.h"
#include "ppbox/cdn/CdnError.h"

#include <ppbox/common/DomainName.h>

#include <framework/string/Parse.h>
#include <framework/logger/StreamRecord.h>
using namespace framework::string;
using namespace framework::logger;
using namespace boost::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Vod2Segment", 0);

#ifndef PPBOX_DNS_VOD_PLAY
#  define PPBOX_DNS_VOD_PLAY "(tcp)(v4)epg.api.pptv.com:80"
#endif

namespace ppbox
{
    namespace cdn
    {

        DEFINE_DOMAIN_NAME(dns_vod_play, PPBOX_DNS_VOD_PLAY);

        PptvVod2::PptvVod2(
            boost::asio::io_service & io_svc)
            : PptvVod(io_svc)
            , open_step_(StepType::not_open)
            , ft_((size_t)-1)
        {
        }

        PptvVod2::~PptvVod2()
        {
        }

        void PptvVod2::set_url(
            framework::string::Url const & url)
        {
            PptvVod::set_url(url);
        }

        void PptvVod2::async_open(
            response_type const & resp)
        {
            assert(StepType::not_open == open_step_);
            set_response(resp);
            boost::system::error_code ec;
            parse_url(ec);
            handle_async_open(ec);
        }

        void PptvVod2::handle_async_open(
            error_code const & ec)
        {
            if (ec) {
                LOG_WARN("play: failure");
                response(ec);
                return;
            }

            switch (open_step_) {
            case StepType::not_open:
                {
                    LOG_INFO("play: start");
                    framework::string::Url url;
                    async_fetch(
                        get_play_url(url),
                        dns_vod_play,
                        play_info_, 
                        boost::bind(&PptvVod2::handle_async_open, this, _1));
                    break;
                }
            case StepType::play:
                {
                    set_user_host(play_info_.uh);
                    deside_ft();
                    std::vector<Vod2Video> & files = play_info_.channel.file;
                    for (size_t i = 0; i < files.size(); ++i) {
                        if (files[i].ft == ft_) {
                            files[i].name = play_info_.channel.nm;
                            files[i].duration = play_info_.channel.dur;
                            set_video(files[i]); // don't use temp variable as param for set_video
                            break;
                        }
                    }
                    for (size_t i = 0; i < play_info_.jumps.size(); ++i) {
                        if (play_info_.jumps[i].ft == ft_) {
                            set_jump(play_info_.jumps[i]);
                            break;
                        }
                    }
                    for (size_t i = 0; i < play_info_.drags.size(); ++i) {
                        if (play_info_.drags[i].ft == ft_) {
                            set_segments(play_info_.drags[i].segments);
                            break;
                        }
                    }
                    response(ec);
                    break;
                }
            default:
                assert(0);
                break;
            }
        }

        void PptvVod2::parse_url(
            boost::system::error_code & ec)
        {
            parse2(url_.param("ft"), ft_);
            ec.clear();
        }

        framework::string::Url & PptvVod2::get_play_url(
            framework::string::Url & url)
        {
            url = url_;
            url.host(dns_vod_play.host());
            url.svc(dns_vod_play.svc());
            url.path("/boxplay.api");
            url.param("id",url_.path().substr(1));
            url.param("auth","55b7c50dc1adfc3bcabe2d9b2015e35c");
            if (ft_ != (-1)) {
                url.param("f", format(ft_));
            }
            LOG_DEBUG("[get_play_url] play url:" << url.to_string());
            return url;
        }

        void PptvVod2::deside_ft()
        {
            std::vector<Vod2Video> & files = play_info_.channel.file;
            for (size_t i = 0; i < files.size(); ++i) {
                if (files[i].ft >= ft_) {
                    ft_ = files[i].ft;
                    break;
                }
            }
            if (ft_ == (size_t)-1) {
                ft_ = files.back().ft;
            }
        }

    } // cdn
} // ppbox
