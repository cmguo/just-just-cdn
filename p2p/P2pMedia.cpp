// P2pMedia.cpp

#include "just/cdn/Common.h"
#include "just/cdn/p2p/P2pMedia.h"
#include "just/cdn/CdnError.h"
#include "just/cdn/p2p/P2pSource.h"

#ifndef JUST_DISABLE_CERTIFY
#  include <just/certify/Certifier.h>
#endif

#include <just/demux/DemuxModule.h>
#include <just/demux/base/DemuxEvent.h>
#include <just/demux/segment/SegmentDemuxer.h>

#include <just/merge/MergeModule.h>
#include <just/merge/Merger.h>

#include <just/data/segment/SegmentSource.h>
#include <just/data/segment/SegmentBuffer.h>

#include <just/common/DynamicString.h>

#include <framework/string/Format.h>
#include <framework/string/Uuid.h>
#include <framework/string/StringToken.h>
using namespace framework::string;
using namespace framework::timer;
#include <framework/logger/StreamRecord.h>

#include <boost/bind.hpp>
#include <boost/ref.hpp>

#ifndef STR_CDN_TYPE
#  define STR_CDN_TYPE "ppsdk"
#endif

#ifndef STR_CDN_PLATFORM
#  define STR_CDN_PLATFORM "just"
#endif

namespace just
{
    namespace cdn
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.cdn.P2pMedia", framework::logger::Debug);

        DEFINE_DYNAMIC_STRING(str_cdn_type, STR_CDN_TYPE);

        DEFINE_DYNAMIC_STRING(str_cdn_platform, STR_CDN_PLATFORM);

        P2pMedia::P2pMedia(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : just::data::SegmentMedia(io_svc, url)
            , url_(url)
            , video_(NULL)
            , jump_(NULL)
            , owner_type_(ot_none)
            , owner_(NULL)
            , source_(NULL)
            , fetch_(new HttpFetch(io_svc))
        {
            parse_url();
        }

        P2pMedia::~P2pMedia()
        {
            fetch_->detach();
        }

        void P2pMedia::parse_url()
        {
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
        }

        void P2pMedia::cancel(
            boost::system::error_code & ec)
        {
            fetch_->cancel(ec);
        }

        void P2pMedia::close(
            boost::system::error_code & ec)
        {
            switch (owner_type_) {
            case ot_demuxer:
                demuxer().status_changed.un(boost::bind(&P2pMedia::on_event, this, _1, _2));
                break;
            default:
                break;
            }
            owner_type_ = ot_none;
        }

        bool P2pMedia::get_basic_info(
            just::avbase::MediaBasicInfo & info, 
            boost::system::error_code & ec) const
        {
            info = parsed_video_;
            ec.clear();
            return true;
        }

        bool P2pMedia::get_info(
            just::avbase::MediaInfo & info,
            boost::system::error_code & ec) const
        {
            info = *video_;
            if (info.type == just::avbase::MediaInfo::live) {
                info.start_time = jump_->server_time.to_time_t();
                info.current = info.shift + (Time::now() - local_time_).total_milliseconds();
            }
            ec.clear();
            return true;
        }

        bool P2pMedia::get_url(
            framework::string::Url & url,
            boost::system::error_code & ec) const
        {
            url = url_;
            ec.clear();
            return true;
        }

        void P2pMedia::set_response(
            MediaBase::response_type const & resp)
        {
            resp_ = resp;
            // it is safe to get user stat object now
            just::demux::DemuxModule & demux = util::daemon::use_module<just::demux::DemuxModule>(get_io_service());
            owner_ = demux.find(*this); // 需要原始的URL
            if (owner_) {
                owner_type_ = ot_demuxer;
                demuxer().status_changed.on(boost::bind(&P2pMedia::on_event, this, _1, _2));
                return;
            }
            just::merge::MergeModule & merge = util::daemon::use_module<just::merge::MergeModule>(get_io_service());
            owner_ = merge.find(*this); // 需要原始的URL
            if (owner_) {
                owner_type_ = ot_merger;
                merger().status_changed.on(boost::bind(&P2pMedia::on_event, this, _1, _2));
                return;
            }
            assert(owner_);
        }

        void P2pMedia::response(
            boost::system::error_code const & ec)
        {
            MediaBase::response_type resp;
            resp.swap(resp_);
            resp(ec);
        }

        void P2pMedia::set_basic_info(
            just::avbase::MediaBasicInfo const & info)
        {
            (just::avbase::MediaBasicInfo &)parsed_video_ = info;
        }

        void P2pMedia::set_video(
            P2pVideo & video)
        {
            (just::avbase::MediaBasicInfo &)video = parsed_video_;
            if (video.type == P2pVideo::live) {
                if (video.shift == 0)
                    video.shift = video.delay;
                if (video.current == 0)
                    video.current = video.shift;
            }
            if (video_ == NULL) {
                video_ = &video;
                parse_video_param(video, MediaBase::url_.param("cdn.video"), false);
                LOG_INFO("[set video] type: " << video_->type);
                LOG_INFO("[set video] flags: " << video_->flags);
                LOG_INFO("[set video] format_type: " << video_->format_type);
                LOG_INFO("[set video] name: " << video_->name);
                LOG_INFO("[set video] file_size: " << video_->file_size);
                LOG_INFO("[set video] duration: " << video_->duration);
                LOG_INFO("[set video] bitrate: " << video_->bitrate);
                if (video_->type == P2pVideo::live) {
                    LOG_INFO("[set video] delay: " << video_->delay);
                    LOG_INFO("[set video] shift: " << video_->shift);
                }
                LOG_INFO("[set video] rid: " << video_->rid);

                url_.path("/" + video_->rid);
            } else {
                assert(*video_ == video);
            }
        }

        void P2pMedia::set_jump(
            P2pJump & jump)
        {
            if (jump_ == NULL) {
                jump_ = &jump;
                parse_jump_param(jump, MediaBase::url_.param("cdn.jump"), true);
                LOG_INFO("[set jump] url: " << jump_->url.host_svc());
                LOG_INFO("[set jump] server_time: " << jump_->server_time.to_time_t());
                LOG_INFO("[set jump] bw_type: " << jump_->bw_type);
                LOG_INFO("[set jump] url2: " << jump_->url2.host_svc());
                local_time_ = local_time_.now();
            } else {
                assert(*jump_ == jump);
            }
        }

        void P2pMedia::set_user_host(
            std::string const & user_host)
        {
            user_host_ = user_host;
            LOG_INFO("[set user_host] " << user_host_);
        }

        void P2pMedia::on_event(
            util::event::Observable const & sender, 
            util::event::Event const & event)
        {
            using just::avbase::StreamStatistic;
            StreamStatistic const & stat = ((just::avbase::StreamEvent const &)event).stat;
            if (stat.status() == StreamStatistic::stream_opening) {
                if (owner_type_ == ot_demuxer) {
                    assert(event == demuxer().status_changed);
                    source_ = 
                        &const_cast<P2pSource &>(static_cast<P2pSource const &>(demuxer().source().source()));
                    source_->p2p_media(*this);
                } else {
                    assert(event == merger().status_changed);
                    source_ = 
                        &const_cast<P2pSource &>(static_cast<P2pSource const &>(merger().source().source()));
                    source_->p2p_media(*this);
                }
            } else if (stat.status() == StreamStatistic::opened) {
                if (!stat.last_error())
                    async_open2();
            } else if (stat.status() == StreamStatistic::closed) {
            }
        }

        bool P2pMedia::parse_jump_param(
            P2pJump & jump, 
            std::string const & param, 
            bool force)
        {
            boost::system::error_code ec;
            ec 
                || ((ec = map_find(param, "url", jump.url, JUST_CDN_PARAM_DELIM)) && !force)
                || ((ec = map_find(param, "url2", jump.url2, JUST_CDN_PARAM_DELIM)) && !force)
                || ((ec = map_find(param, "svrtime", jump.server_time, JUST_CDN_PARAM_DELIM)) && !force)
                || (ec = map_find(param, "bwtype", jump.bw_type, JUST_CDN_PARAM_DELIM));
            return !ec;
        }

        bool P2pMedia::parse_video_param(
            P2pVideo & video, 
            std::string const & param, 
            bool force)
        {
            boost::system::error_code ec;
            ec 
                || ((ec = map_find(param, "name", video.name, JUST_CDN_PARAM_DELIM)) && !force)
                || ((ec = map_find(param, "bitrate", video.bitrate, JUST_CDN_PARAM_DELIM)) && !force)
                || (ec = map_find(param, "duration", video.duration, JUST_CDN_PARAM_DELIM));
            return !ec;
        }

        void P2pMedia::async_fetch(
            framework::string::Url const & url, 
            framework::network::NetName const & server_host, 
            parser_t parser, 
            void * t, 
            HttpFetch::response_type const & resp)
        {
            LOG_INFO("[async_fetch] start, url: " << url.to_string());

            fetch_->async_fetch(url, server_host, 
                boost::bind(&P2pMedia::handle_fetch, this, _1, parser, t, resp));
        }

        void P2pMedia::handle_fetch(
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
} // namespace just
