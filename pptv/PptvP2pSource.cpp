// PptvP2pSource.cpp

#include "just/cdn/Common.h"
#include "just/cdn/pptv/PptvP2pSource.h"
#include "just/cdn/pptv/PptvMedia.h"

#include <just/demux/segment/SegmentDemuxer.h>

#ifndef JUST_DISABLE_MERGE
#include <just/merge/Merger.h>
#endif

#include <just/data/segment/SegmentSource.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>

#include <boost/bind.hpp>

namespace just
{
    namespace cdn
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("just.cdn.PptvP2pSource", framework::logger::Debug);

        PptvP2pSource::PptvP2pSource(
            boost::asio::io_service & io_svc)
            : HttpSource(io_svc)
            , pptv_media_(NULL)
            , seg_source_(NULL)
        {
        }

        PptvP2pSource::~PptvP2pSource()
        {
        }

        bool PptvP2pSource::open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            boost::system::error_code & ec)
        {
            LOG_DEBUG("[open] url:" << url.to_string()
                <<" range: "<< beg << " - " << end);

            open_log(false);

            framework::string::Url p2p_url(url);
            if (!prepare(p2p_url, beg, end, ec))
                return false;
            LOG_DEBUG("[open] p2p_url:" << p2p_url.to_string()
                <<" range: "<< beg << " - " << end);
            return HttpSource::open(p2p_url, beg, end, ec);
        }

        void PptvP2pSource::async_open(
            framework::string::Url const & url,
            boost::uint64_t beg, 
            boost::uint64_t end, 
            response_type const & resp)
        {
            LOG_DEBUG("[async_open] url:" << url.to_string()
                <<" range: "<< beg << " - " << end);

            open_log(false);

            framework::string::Url p2p_url(url);
            boost::system::error_code ec;
            if (!prepare(p2p_url, beg, end, ec)) {
                get_io_service().post(
                    boost::bind(resp, ec));
            } else {
                LOG_DEBUG("[async_open] p2p_url:" << p2p_url.to_string()
                    <<" range: "<< beg << " - " << end);
                HttpSource::async_open(p2p_url, beg, end, resp);
            }
        }

        bool PptvP2pSource::close(
            boost::system::error_code & ec)
        {
            open_log(true);
            return HttpSource::close(ec);
        }

        just::cdn::HttpStatistics const & PptvP2pSource::http_stat() const
        {
            if (http_stat_.not_ended) {
                const_cast<PptvP2pSource *>(this)->open_log(true);
            }
            return http_stat_;
        }

        void PptvP2pSource::pptv_media(
            just::cdn::PptvMedia const & media)
        {
            pptv_media_ = &media;

            switch (pptv_media().owner_type()) {
                case just::cdn::PptvMedia::ot_demuxer:
                    pptv_media().demuxer().buffer_update.on(boost::bind(&PptvP2pSource::on_event, this, _1, _2));
                    seg_source_ = &pptv_media().demuxer().source();
                    break;
#ifndef JUST_DISABLE_MERGE
                case just::cdn::PptvMedia::ot_merger:
                     //pptv_media().merger().on<just::demux::BufferingEvent>(boost::bind(&PptvP2pSource::on_event, this, _1));
                    seg_source_ = &pptv_media().merger().source();
                    break;
#endif
                default:
                    assert(0);
                    break;
            }

            parse_param(pptv_media().p2p_params());
        }

        void PptvP2pSource::on_event(
            util::event::Observable const & sender, 
            util::event::Event const & event)
        {
            on_stream_status(((just::avbase::StreamEvent const &)event).stat);
        }

        void PptvP2pSource::open_log(
            bool end)
        {
            if (!end) {
                http_stat_.begin_try();
            } else {
                http_stat_.end_try(http_.stat());
                if (http_stat_.try_times == 1)
                    http_stat_.response_data_time = http_stat_.total_elapse;
            }
        }

    } // namespace peer
} // namespace just
