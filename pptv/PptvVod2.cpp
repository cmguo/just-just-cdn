//PptvVod2.cpp

#include "just/cdn/Common.h"
#include "just/cdn/pptv/PptvVod2.h"
#include "just/cdn/CdnError.h"

#include <just/common/DomainName.h>
#include <just/common/UrlHelper.h>

#include <util/archive/ArchiveBuffer.h>

#include <framework/string/Parse.h>
#include <framework/logger/StreamRecord.h>
using namespace framework::string;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.cdn.PptvVod2", framework::logger::Debug);

#ifndef JUST_DNS_VOD_PLAY
#  define JUST_DNS_VOD_PLAY "(tcp)(v4)epg.api.pptv.com:80"
#endif

namespace just
{
    namespace cdn
    {

        DEFINE_DOMAIN_NAME(dns_vod_play, JUST_DNS_VOD_PLAY);

        PptvVod2::PptvVod2(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : PptvVod(io_svc, url)
            , open_step_(StepType::closed)
            , ft_((size_t)-1)
        {
        }

        PptvVod2::~PptvVod2()
        {
        }

        void PptvVod2::async_open(
            response_type const & resp)
        {
            assert(StepType::closed == open_step_);
            set_response(resp);
            boost::system::error_code ec;
            parse_url(ec);
            handle_async_open(ec);
        }

        void PptvVod2::async_open2()
        {
            boost::system::error_code ec;
            handle_async_open(ec);
        }

        void PptvVod2::handle_async_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                LOG_WARN("play: failure");
                response(ec);
                return;
            }

            framework::string::Url url;

            switch (open_step_) {
                case StepType::closed:
                    if (jump_ && owner_type() == ot_demuxer) {
                        open_step_ = StepType::wait2;
                        response(ec);
                        break;
                    }
                case StepType::wait2:
                    open_step_ = StepType::playing;
                    LOG_INFO("play: start");
                    async_fetch(
                        get_play_url(url),
                        dns_vod_play,
                        play_info_, 
                        boost::bind(&PptvVod2::handle_async_open, this, _1));
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
                    //assert(0);
                    break;
            }
        }

        void PptvVod2::parse_url(
            boost::system::error_code & ec)
        {
            parse2(url_.param("ft"), ft_);
            if (just::common::decode_param(url_, "play_xml", ec)) {
                std::string play_xml = url_.param("play_xml");
                util::archive::ArchiveBuffer<> buf(boost::asio::buffer(play_xml));
                util::archive::XmlIArchive<> ia(buf);
                ia >> play_info_;
                if (ia) {
                    open_step_ = StepType::playing;
                }
            }
            url_.param("play_xml", "");
            ec.clear();
        }

        framework::string::Url & PptvVod2::get_play_url(
            framework::string::Url & url)
        {
            url = url_;
            if (url_.host().empty()) {
                url.host(dns_vod_play.host());
                url.svc(dns_vod_play.svc());
            } else {
                url.host(url_.host());
                url.svc(url_.svc());
            }
            url.path("/boxplay.api");
            url.param("id", url_.path().substr(1));
            url.param_add("content", "need_drag");
            if (ft_ != (size_t)-1) {
                url.param("f", format(ft_));
            }
            LOG_DEBUG("[get_play_url] play url:" << url.to_string());
            return url;
        }

        void PptvVod2::deside_ft(
            boost::system::error_code & ec)
        {
            std::vector<Vod2Video> & files = play_info_.channel.file;
            if (files.size() == 0) {
                ec = error::bad_file_format;
                return;
            }
            std::sort(files.begin(), files.end());
            Vod2Video * video = NULL;
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
            video->duration = play_info_.channel.dur;
            set_video(*video); // don't use temp variable as param for set_video

            bool failed = true;
            for (size_t i = 0; i < play_info_.jumps.size(); ++i) {
                if (play_info_.jumps[i].ft == ft_) {
                    set_jump(play_info_.jumps[i]);
                    failed = false;
                    break;
                }
            }
            if (failed) {
                ec = error::bad_ft_param;
                return;
            }
            failed = true;
            for (size_t i = 0; i < play_info_.drags.size(); ++i) {
                    if (play_info_.drags[i].ft == ft_) {
                    set_segments(play_info_.drags[i].segments);
                    failed = false;
                    break;
                }
            }
            if (failed) {
                ec = error::bad_ft_param;
                return;
            }
        }

    } // cdn
} // just
