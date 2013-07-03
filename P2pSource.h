// P2pSource.h

#ifndef PPBOX_CDN_P2P_SOURCE_H_
#define PPBOX_CDN_P2P_SOURCE_H_

#include <ppbox/cdn/HttpStatistics.h>

#include <ppbox/data/source/HttpSource.h>

#include <util/event/Event.h>

#include <framework/string/Url.h>

namespace ppbox
{
    namespace data
    {
        class SegmentSource;
    }

    namespace demux
    {
        class DemuxStatistic;
    }

    namespace cdn
    {

        class PptvMedia;

        class P2pSource
            : public  ppbox::data::HttpSource
        {
        public:
            P2pSource(
                boost::asio::io_service & io_svc);

            virtual ~P2pSource();

        public:
            ppbox::cdn::HttpStatistics const & http_stat() const;

            void pptv_media(
                PptvMedia const & media);

            ppbox::cdn::PptvMedia const & pptv_media()
            {
                return *pptv_media_;
            }

        protected:
            void open_log(
                bool end);

            ppbox::data::SegmentSource const & seg_source()
            {
                return *seg_source_;
            }

        private:
            void on_event(
                util::event::Observable const & sender, 
                util::event::Event const & event);

            virtual void on_demux_stat(
                ppbox::demux::DemuxStatistic const & stat) = 0;

            virtual void parse_param(
                std::string const & params) = 0;

        private:
            ppbox::cdn::PptvMedia const * pptv_media_;
            ppbox::data::SegmentSource const * seg_source_;
            ppbox::cdn::HttpStatistics http_stat_;
        };

    } // namespace peer
} // namespace ppbox

#endif // PPBOX_PEER_PEER_SOURCE_H_
