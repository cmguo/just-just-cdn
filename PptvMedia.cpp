// PptvMedia.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/PptvMedia.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/P2pSource.h"

#include <ppbox/certify/Certifier.h>
#include <ppbox/dac/DacModule.h>
#include <ppbox/dac/DacInfoPlayOpen.h>
#include <ppbox/dac/DacInfoPlayClose.h>

#include <ppbox/demux/DemuxModule.h>
#include <ppbox/demux/base/DemuxEvent.h>
#include <ppbox/demux/segment/SegmentDemuxer.h>

#include <ppbox/merge/MergeModule.h>
#include <ppbox/merge/MergerBase.h>

#include <ppbox/data/segment/SegmentSource.h>
#include <ppbox/data/segment/SegmentBuffer.h>

#include <ppbox/common/DynamicString.h>

#include <util/protocol/pptv/TimeKey.h> // for gen_key_from_time

#include <framework/string/Format.h>
#include <framework/string/Uuid.h>
#include <framework/string/StringToken.h>
using namespace framework::string;
#include <framework/logger/StreamRecord.h>

#include <boost/bind.hpp>
#include <boost/ref.hpp>

#ifndef STR_CDN_TYPE
#  define STR_CDN_TYPE "ppsdk"
#endif

#ifndef STR_CDN_PLATFORM
#  define STR_CDN_PLATFORM "ppbox"
#endif

namespace ppbox
{
    namespace cdn
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.cdn.PptvMedia", framework::logger::Debug);

        DEFINE_DYNAMIC_STRING(str_cdn_type, STR_CDN_TYPE);

        DEFINE_DYNAMIC_STRING(str_cdn_platform, STR_CDN_PLATFORM);

        PptvMedia::PptvMedia(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : ppbox::data::SegmentMedia(io_svc, url)
#ifndef PPBOX_DISABLE_CERTIFY
            , cert_(util::daemon::use_module<ppbox::certify::Certifier>(io_svc))
#else
            , cert_(*(ppbox::certify::Certifier *)NULL)
#endif
#ifndef PPBOX_DISABLE_DAC
            , dac_(util::daemon::use_module<ppbox::dac::DacModule>(io_svc))
#else
            , dac_(*(ppbox::dac::DacModule *)NULL)
#endif
            , url_(url)
            , video_(NULL)
            , jump_(NULL)
            , owner_type_(ot_none)
            , owner_(NULL)
            , fetch_(new HttpFetch(io_svc))
        {
            parse_url();
        }

        PptvMedia::~PptvMedia()
        {
            fetch_->detach();
        }

        void PptvMedia::parse_url()
        {
            /*
             * 补充vvid、type、platform
             * cdn.jump.bwtype也在PptvMedia统一处理
             * cdn.jump.server_host 也在PptvMedia统一处理
             * cdn.drag.* 由派生类处理
             * cdn.* 
             * p2p.* 
             */
            url_.protocol("http");

            if (parse_video_param(parsed_video_, url_.param("cdn.video"))) {
                set_video(parsed_video_);
            }
            url_.param("cdn.video", ""); // clear

            if (parse_jump_param(parsed_jump_, url_.param("cdn.jump"))) {
                set_jump(parsed_jump_);
            }
            url_.param("cdn.jump", ""); // clear

            p2p_params_ = url_.param("p2p");
            url_.param("p2p", "");

            if (url_.param("type").empty()) {
                url_.param("type", str_cdn_type);
            }
            if (url_.param("platform").empty()) {
                url_.param("platform", str_cdn_platform);
            }
            if (url_.param("auth").empty()) {
                url_.param("auth", "55b7c50dc1adfc3bcabe2d9b2015e35c");
            }
            if (url_.param("vvid").empty()) {
                framework::string::Uuid vvid;
                vvid.generate();
                url_.param("vvid", vvid.to_string());
            }
        }

        void PptvMedia::cancel(
            boost::system::error_code & ec)
        {
            fetch_->cancel(ec);
        }

        void PptvMedia::close(
            boost::system::error_code & ec)
        {
            switch (owner_type_) {
                case ot_demuxer:
                    demuxer().status_changed.un(boost::bind(&PptvMedia::on_event, this, _1, _2));
                    break;
                default:
                    break;
            }
            owner_type_ = ot_none;
        }

        bool PptvMedia::get_basic_info(
            ppbox::data::MediaBasicInfo & info, 
            boost::system::error_code & ec) const
        {
            info = parsed_video_;
            ec.clear();
            return true;
        }

        bool PptvMedia::get_info(
            ppbox::data::MediaInfo & info,
            boost::system::error_code & ec) const
        {
            info = *video_;
            if (info.type == ppbox::data::MediaInfo::live) {
                info.start_time = jump_->server_time.to_time_t();
                info.current = info.shift + (Time::now() - local_time_).total_milliseconds();
            }
            ec.clear();
            return true;
        }

        bool PptvMedia::get_url(
            framework::string::Url & url,
            boost::system::error_code & ec) const
        {
            url = url_;
            ec.clear();
            return true;
        }

        void PptvMedia::set_response(
            MediaBase::response_type const & resp)
        {
            resp_ = resp;
            // it is safe to get user stat object now
            ppbox::demux::DemuxModule & demux = util::daemon::use_module<ppbox::demux::DemuxModule>(get_io_service());
            owner_ = demux.find(*this); // 需要原始的URL
            if (owner_) {
                owner_type_ = ot_demuxer;
                P2pSource & peer = 
                    const_cast<P2pSource &>(static_cast<P2pSource const &>(demuxer().source().source()));
                peer.pptv_media(*this);
                demuxer().status_changed.on(boost::bind(&PptvMedia::on_event, this, _1, _2));
                return;
            }
            ppbox::merge::MergeModule & merge = util::daemon::use_module<ppbox::merge::MergeModule>(get_io_service());
            owner_ = merge.find(*this); // 需要原始的URL
            if (owner_) {
                owner_type_ = ot_merger;
                P2pSource & peer = 
                    const_cast<P2pSource &>(static_cast<P2pSource const &>(merger().source().source()));
                peer.pptv_media(*this);
                //merger().on<ppbox::demux::StatusChangeEvent>(boost::bind(&PptvMedia::on_event, this, _1));
                return;
            }
            assert(owner_);
        }

        void PptvMedia::response(
            boost::system::error_code const & ec)
        {
            MediaBase::response_type resp;
            resp.swap(resp_);
            resp(ec);
        }

        void PptvMedia::set_basic_info(
            ppbox::data::MediaBasicInfo const & info)
        {
            (ppbox::data::MediaBasicInfo &)parsed_video_ = info;
        }

        void PptvMedia::set_video(
            Video & video)
        {
            (ppbox::data::MediaBasicInfo &)video = parsed_video_;
            if (video.type == Video::live) {
                if (video.shift == 0)
                    video.shift = video.delay;
                if (video.current == 0)
                    video.current = video.shift;
            }
            if (video_ == NULL) {
                video_ = &video;
                LOG_INFO("[set video] name: " << video_->name);
                LOG_INFO("[set video] duration: " << video_->duration);
                if (video_->type == Video::live) {
                    LOG_INFO("[set video] delay: " << video_->delay);
                    LOG_INFO("[set video] shift: " << video_->shift);
                }
                LOG_INFO("[set video] rid: " << video_->rid);

                url_.path("/" + video_->rid);
            } else {
                assert(*video_ == video);
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
                local_time_ = local_time_.now();

                url_.host(jump_->server_host.host());
                url_.svc(jump_->server_host.svc());
            } else {
                assert(*jump_ == jump);
            }
        }

        void PptvMedia::set_user_host(
            std::string const & user_host)
        {
            user_host_ = user_host;
            LOG_INFO("[set user_host] " << user_host_);
        }
        
        void PptvMedia::on_event(
            util::event::Observable const & sender, 
            util::event::Event const & event)
        {
            if (owner_type_ == ot_demuxer) {
                assert(event == demuxer().status_changed);
                using ppbox::demux::DemuxStatistic;
                DemuxStatistic const & stat = demuxer().status_changed.stat;
                if (stat.state() == DemuxStatistic::opened) {
                    P2pSource const & source_ = (P2pSource const &)demuxer().source().source();
                    dac_.submit(ppbox::dac::DacPlayOpenInfo(*this, source_, stat));
                    if (!stat.last_error())
                        async_open2();
                } else if (stat.state() == DemuxStatistic::stopped) {
                    dac_.submit(ppbox::dac::DacPlayCloseInfo(*this, stat, demuxer().source()));
                }
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
                (boost::uint32_t)(jump_->server_time.to_time_t() + (time_t)(Time::now() - local_time_).total_seconds()));
        }

        void PptvMedia::async_fetch(
            framework::string::Url const & url, 
            framework::network::NetName const & server_host, 
            parser_t parser, 
            void * t, 
            HttpFetch::response_type const & resp)
        {
            LOG_INFO("[async_fetch] start, url: " << url.to_string());

            fetch_->async_fetch(url, server_host, 
                boost::bind(&PptvMedia::handle_fetch, this, _1, parser, t, resp));
        }

        void PptvMedia::handle_fetch(
            boost::system::error_code const & ecc, 
            parser_t parser, 
            void * t, 
            HttpFetch::response_type const & resp)
        {
            boost::system::error_code ec = ecc;
            open_logs_.push_back(fetch_->http_stat());

            if (!ec) {
                parser(*fetch_, t, ec);
            }

            std::string path = fetch_->http_request().head().path;
            path = path.substr(0, path.find('?'));
            if (ec) {
                LOG_WARN("[handle_fetch] failed, path: " << path 
                    << " ec: " << ec.message() << " elapse: " << fetch_->http_stat().total_elapse << " milliseconds");
                open_logs_.back().last_last_error = ec;
            } else {
                LOG_INFO("[handle_fetch] succeed, path: " << path 
                    << " elapse: " << fetch_->http_stat().total_elapse << " milliseconds");
            }

            boost::system::error_code ec2;
            fetch_->close(ec2);

            resp(ec);
        }


    } // namespace cdn
} // namespace ppbox
