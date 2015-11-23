//TripMedia.cpp

#include "just/cdn/Common.h"
#include "just/cdn/trip/TripMedia.h"
#include "just/cdn/CdnError.h"

#include <just/common/DomainName.h>
#include <just/common/UrlHelper.h>
#include <just/trip/TripModule.h>

#include <util/archive/ArchiveBuffer.h>
#include <util/daemon/Daemon.h>

#include <framework/string/Parse.h>
#include <framework/logger/StreamRecord.h>
#include <framework/system/LogicError.h>
using namespace framework::string;
using namespace framework::system;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.cdn.TripMedia", framework::logger::Debug);

#ifndef JUST_DNS_VOD_PLAY
#  define JUST_DNS_VOD_PLAY "(tcp)(v4)epg.api.trip.com:80"
#endif

namespace just
{
    namespace cdn
    {

        DEFINE_DOMAIN_NAME(dns_index, JUST_DNS_VOD_PLAY);

        TripMedia::TripMedia(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
            : P2pMedia(io_svc, url)
            , open_step_(StepType::closed)
        {
        }

        TripMedia::~TripMedia()
        {
        }

        size_t TripMedia::segment_count() const
        {
            size_t ret = size_t(-1);
            ret = (size_t)meta_info_.segcount;
            return ret;
        }

        bool TripMedia::segment_url(
            size_t segment, 
            framework::string::Url & url,
            boost::system::error_code & ec) const
        {
            if (segment >= meta_info_.segcount) {
                ec = logic_error::item_not_exist;
                return false;
            }
            url = url_;
            url.protocol("http");
            url.path("/fetch/" + format(segment) + url.path());
            ec.clear();
            return true;
        }

        bool TripMedia::segment_info(
            size_t segment,
            just::data::SegmentInfo & info,
            boost::system::error_code & ec) const
        {
            ec.clear();
            if (segment >= meta_info_.segcount) {
                ec = logic_error::item_not_exist;
                return false;
            }
            info.head_size = boost::uint64_t(-1);
            info.offset = boost::uint64_t(-1);
            if (index_info_.segments.is_initialized() && segment < index_info_.segments.get().size()) {
                ::trip::client::SegmentMeta const & seg(index_info_.segments.get().at(segment));
                info.size = seg.bytesize;
                info.duration = seg.duration;
            } else {
                info.size = boost::uint64_t(-1);
                info.duration = boost::uint64_t(-1);
            }
            return true;
        }

        void TripMedia::async_open(
            response_type const & resp)
        {
            assert(StepType::closed == open_step_);
            set_response(resp);
            boost::system::error_code ec;
            parse_url(ec);
            handle_async_open(ec);
        }

        void TripMedia::handle_async_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                LOG_WARN("open: failure");
                response(ec);
                if (open_step_ != StepType::p2p_meta) {
                    return;
                }
            }

            framework::string::Url url;

            switch (open_step_) {
                case StepType::closed:
#ifndef JUST_DISABLE_TRIP
                    open_step_ = StepType::p2p_meta;
                    LOG_INFO("p2p_meta: start");
                    {
                        framework::network::NetName dns_vod("127.0.0.1:4444");
                        async_fetch(
                            get_p2p_url(url),
                            dns_vod,
                            meta_info_, 
                            boost::bind(&TripMedia::handle_async_open, this, _1));
                    }
#else
                    handle_async_open(logic_error::not_supported);
#endif
                    break;
                case StepType::p2p_meta:
                    if (ec) {
                        open_step_ = StepType::cdn_index;
                        async_fetch(
                            get_index_url(url),
                            dns_index,
                            index_info_, 
                            boost::bind(&TripMedia::handle_async_open, this, _1));
                    } else {
                        P2pVideo video;
                        meta_to_video(meta_info_, video);
                        set_video(video);
                        open_step_ = StepType::finish;
                        response(ec);
                    }
                    break;
                case StepType::cdn_index:
                    {
                        P2pVideo video;
                        index_to_video(index_info_, video);
                        set_video(video);
                        P2pJump jump;
                        index_to_jump(index_info_, jump);
                        set_jump(jump);
                        open_step_ = StepType::finish;
                        response(ec);
                    }
                    break;
                default:
                    //assert(0);
                    break;
            }
        }

        void TripMedia::parse_url(
            boost::system::error_code & ec)
        {
        }

        framework::string::Url & TripMedia::get_p2p_url(
            framework::string::Url & url)
        {
#ifndef JUST_DISABLE_TRIP
            just::trip::TripModule & trip =
                util::daemon::use_module<just::trip::TripModule>(get_io_service());
            return trip.get_p2p_url(url_, url);
#else
            return url;
#endif
        }

        framework::string::Url & TripMedia::get_index_url(
            framework::string::Url & url)
        {
            return url;
        }

    } // cdn
} // just
