// PptvLive3.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/PptvLive3.h"
#include "ppbox/cdn/CdnError.h"

#include <ppbox/common/DomainName.h>

#include <framework/string/Format.h>
using namespace framework::string;
#include <framework/logger/StreamRecord.h>

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.cdn.PptvLive3", framework::logger::Debug);

#ifndef PPBOX_DNS_LIVE2_PLAY
#  define PPBOX_DNS_LIVE2_PLAY "(tcp)(v4)epg.api.pptv.com:80"
#endif

namespace ppbox
{
    namespace cdn
    {

        DEFINE_DOMAIN_NAME(dns_live2_play, PPBOX_DNS_LIVE2_PLAY);

        PptvLive3::PptvLive3(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : PptvLive(io_svc, url)
            , open_step_(StepType::closed)
        {
        }

        PptvLive3::~PptvLive3()
        {
        }

        void PptvLive3::async_open(
            response_type const & resp)
        {
            assert(StepType::closed == open_step_);

            set_response(resp);
            boost::system::error_code ec;
            parse_url(ec);
            handle_async_open(ec);
        }

        void PptvLive3::handle_async_open(
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
                    open_step_ = StepType::playing;
                    LOG_INFO("jump: start");
                    async_fetch(
                        get_play_url(url),
                        dns_live2_play,
                        play_info_, 
                        boost::bind(&PptvLive3::handle_async_open, this ,_1));
                    break;
                case StepType::playing:
                    {
                        boost::system::error_code ec;
                        deside_ft(ec);
                        set_user_host(play_info_.uh);
                        open_step_ = StepType::finish;
                        response(ec);
                        break;
                    }
                default:
                    assert(0);
                    break;
            }
        }

        void PptvLive3::parse_url(
            boost::system::error_code & ec)
        {
            parse2(url_.param("ft"), ft_);
            ec.clear();
        }

        framework::string::Url&  PptvLive3::get_play_url(
            framework::string::Url & url) const
        {
            url = url_;
            url.host(dns_live2_play.host());
            url.svc(dns_live2_play.svc());
            url.path("/boxplay.api");
            url.param("id", url_.path().substr(1));
            if (ft_ != (size_t)-1) {
                url.param("f", format(ft_));
            }
            return url;
        }

        void PptvLive3::deside_ft(
            boost::system::error_code & ec)
        {
            std::vector<Live3Video> & files = play_info_.channel.stream.item;
            if (files.size() == 0) {
                ec = error::bad_file_format;
                return;
            }
            std::sort(files.begin(), files.end());
            Live3Video * video = NULL;
            for (size_t i = 0; i < files.size(); ++i) {
                if (files[i].ft >= ft_) {
                    video = &files[i];
                    break;
                }
            }
            if (video == NULL) {
                video = &files.back();
            }
            ft_ = video->ft;
            video->name = play_info_.channel.nm;
            video->shift = play_info_.channel.stream.jump;
            video->delay = play_info_.channel.stream.delay;
            set_video(*video);
            set_jump(play_info_.jump);
            set_segment(play_info_.channel.stream.seg);
        }

    } // namespace cdn
} // namespace ppbox

// PptvLive3.cpp
