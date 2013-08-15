// P2pSource.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/P2pSource.h"
#include "ppbox/cdn/PptvMedia.h"

#include <ppbox/demux/base/DemuxEvent.h>
#include <ppbox/demux/segment/SegmentDemuxer.h>

#include <ppbox/merge/MergerBase.h>

#include <ppbox/data/segment/SegmentSource.h>

#include <framework/logger/Logger.h>
#include <framework/logger/StreamRecord.h>
using namespace framework::string;

namespace ppbox
{
    namespace cdn
    {

        FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("ppbox.peer.P2pSource", framework::logger::Debug);

        P2pSource::P2pSource(
            boost::asio::io_service & io_svc)
            : HttpSource(io_svc)
            , pptv_media_(NULL)
            , seg_source_(NULL)
        {
        }

        P2pSource::~P2pSource()
        {
        }

        ppbox::cdn::HttpStatistics const & P2pSource::http_stat() const
        {
            if (http_stat_.try_times > 0) {
                const_cast<P2pSource *>(this)->open_log(true);
            }
            return http_stat_;
        }

        void P2pSource::pptv_media(
            ppbox::cdn::PptvMedia const & media)
        {
            pptv_media_ = &media;

            switch (pptv_media().owner_type()) {
                case ppbox::cdn::PptvMedia::ot_demuxer:
                    pptv_media().demuxer().buffer_update.on(boost::bind(&P2pSource::on_event, this, _1, _2));
                    seg_source_ = &pptv_media().demuxer().source();
                    break;
                case ppbox::cdn::PptvMedia::ot_merger:
                     //pptv_media().merger().on<ppbox::demux::BufferingEvent>(boost::bind(&P2pSource::on_event, this, _1));
                    seg_source_ = &pptv_media().merger().source();
                    break;
                default:
                    assert(0);
                    break;
            }

            parse_param(pptv_media().p2p_params());
        }

        void P2pSource::on_event(
            util::event::Observable const & sender, 
            util::event::Event const & event)
        {
            if (pptv_media().owner_type() == ppbox::cdn::PptvMedia::ot_demuxer) {
                ppbox::demux::SegmentDemuxer & demuxer = pptv_media().demuxer();
                on_demux_stat(demuxer.buffer_update.stat);
            }
        }

        void P2pSource::open_log(
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
} // namespace ppbox
