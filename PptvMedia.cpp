// PptvMedia.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/PptvMedia.h"
#include "ppbox/cdn/CdnError.h"

#include <ppbox/certify/Certifier.h>
#include <ppbox/dac/DacModule.h>
#include <ppbox/dac/DacInfoPlayOpen.h>
#include <ppbox/dac/DacInfoPlayClose.h>

#include <ppbox/demux/DemuxModule.h>
#include <ppbox/demux/base/DemuxEvent.h>
#include <ppbox/demux/base/BufferDemuxer.h>

#include <ppbox/common/DynamicString.h>

#include <util/protocol/pptv/TimeKey.h> // for gen_key_from_time

#include <framework/string/Format.h>
#include <framework/string/Uuid.h>
#include <framework/string/StringToken.h>
using namespace framework::string;
#include <framework/logger/StreamRecord.h>
using namespace framework::logger;

#include <boost/bind.hpp>

#ifndef STR_CDN_TYPE
#  define STR_CDN_TYPE "ppsdk"
#endif

#ifndef STR_CDN_PLATFORM
#  define STR_CDN_PLATFORM "ppsdk"
#endif

namespace ppbox
{
    namespace cdn
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("PptvMedia", 0);

        DEFINE_DYNAMIC_STRING(str_cdn_type, STR_CDN_TYPE);

        DEFINE_DYNAMIC_STRING(str_cdn_platform, STR_CDN_PLATFORM);

        PptvMedia::PptvMedia(
            boost::asio::io_service & io_svc)
            : ppbox::data::MediaBase(io_svc)
            , cert_(util::daemon::use_module<ppbox::certify::Certifier>(io_svc))
            , dac_(util::daemon::use_module<ppbox::dac::DacModule>(io_svc))
            , demuxer_(NULL)
            , video_(NULL)
            , jump_(NULL)
            , fetch_(new HttpFetch(io_svc))
        {
        }

        PptvMedia::~PptvMedia()
        {
            if (demuxer_) {
                demuxer_->un<ppbox::demux::StatusChangeEvent>(boost::bind(&PptvMedia::on_event, this, _1));
            }
            fetch_->detach();
        }

        void PptvMedia::set_url(
            framework::string::Url const &url)
        {
            /*
             * 补充vvid、type、platform
             * cdn.jump.bwtype也在PptvMedia统一处理
             * cdn.jump.server_host 也在PptvMedia统一处理
             * cdn.drag.* 由派生类处理
             * cdn.* 
             * p2p.* 
             */
            ppbox::data::MediaBase::set_url(url);

            if (parse_jump_param(parsed_jump_, url_.param("cdn.jump"))) {
                jump_ = &parsed_jump_;
            }
            url_.param("cdn.jump", ""); // clear

            if (parse_video_param(parsed_video_, url_.param("cdn.video"))) {
                video_ = &parsed_video_;
            }
            url_.param("cdn.video", ""); // clear

            p2p_params_ = url_.param("p2p");
            url_.param("p2p", "");

            if (url_.param("type").empty()) {
                url_.param("type", str_cdn_type);
            }
            if (url_.param("platform").empty()) {
                url_.param("platform", str_cdn_platform);
            }
            if (url_.param("vvid").empty()) {
                framework::string::Uuid vvid;
                vvid.generate();
                url_.param("vvid", vvid.to_string());
            }
        }

        void PptvMedia::cancel()
        {
            fetch_->cancel();
        }

        void PptvMedia::close()
        {
        }

        boost::system::error_code PptvMedia::get_info(
            ppbox::data::MediaInfo & info,
            boost::system::error_code & ec)
        {
            info.name = video_->name;
            info.bitrate = video_->bitrate;
            info.duration = video_->duration;
            info.is_live = (0 != video_->duration);
            ec.clear();
            return ec;
        }

        void PptvMedia::set_response(
            MediaBase::response_type const & resp)
        {
            resp_ = resp;
            // it is safe to get user stat object now
            //if (demuxer_) {
            //    demuxer_->on<ppbox::demux::StatusChangeEvent>(boost::bind(&PptvMedia::on_event, this, _1));
            //}
        }

        void PptvMedia::response(
            boost::system::error_code const & ec)
        {
            MediaBase::response_type resp;
            resp.swap(resp_);
            resp(ec);
        }

        void PptvMedia::set_video(
            Video & video)
        {
            if (video_ == NULL) {
                video_ = &video;
                LOG_INFO("[set video] name: " << video_->name);
                LOG_INFO("[set video] duration: " << video_->duration);
                if (video_->is_live)
                    LOG_INFO("[set video] delay: " << video_->delay);
                LOG_INFO("[set video] rid: " << video_->rid);
            }
        }

        void PptvMedia::set_jump(
            Jump & jump)
        {
            if (jump_ == NULL) {
                jump_ = &jump;
                LOG_INFO("[set jump] server_host: " << jump_->server_host.host_svc());
                LOG_INFO("[set jump] server_time: " << jump_->server_time.to_time_t());
                LOG_INFO("[set jump] bw_type: " << jump_->bw_type);
                LOG_INFO("[set jump] back_host: " << jump_->back_host.host_svc());
            }
        }

        void PptvMedia::set_user_host(
            std::string const & user_host)
        {
            user_host_ = user_host;
            LOG_INFO("[set user_host] " << user_host_);
        }
        
        void PptvMedia::on_event(
            util::event::Event const & e)
        {
            ppbox::demux::StatusChangeEvent const & event = *e.cast<ppbox::demux::StatusChangeEvent>();
            if (event.stat.state() == ppbox::demux::DemuxStatistic::opened) {
                ppbox::peer::PeerSource * source_ = NULL; // ((BufferDemuxer *)user_stat_)->source();
                dac_.submit(ppbox::dac::DacPlayOpenInfo(*this, *source_, event.stat));
                if (!event.stat.last_error())
                    async_open2();
            } else if (event.stat.state() == ppbox::demux::DemuxStatistic::stopped) {
                dac_.submit(ppbox::dac::DacPlayCloseInfo(*this, event.stat, demuxer_->buffer_stat()));
            }
        }

        bool PptvMedia::parse_jump_param(
            Jump & jump, 
            std::string const & param)
        {
            boost::system::error_code ec;
            ec 
                || (ec = map_find(param, "svrhost", jump.server_host, "&"))
                || (ec = map_find(param, "svrtime", jump.server_time, "&"))
                || (ec = map_find(param, "bakhost", jump.back_host, "&"))
                || (ec = map_find(param, "bwtype", jump.bw_type, "&"));
            return !ec;
        }

        bool PptvMedia::parse_video_param(
            Video & video, 
            std::string const & param)
        {
            boost::system::error_code ec;
            ec 
                || (ec = map_find(param, "name", video.name, "&"))
                || (ec = map_find(param, "bitrate", video.bitrate, "&"))
                || (ec = map_find(param, "duration", video.duration, "&"));
            return !ec;
        }

        std::string PptvMedia::get_key() const
        {
            return util::protocol::pptv::gen_key_from_time(
                jump_->server_time.to_time_t() + (Time::now() - local_time_).total_seconds());
        }

    } // namespace cdn
} // namespace ppbox
