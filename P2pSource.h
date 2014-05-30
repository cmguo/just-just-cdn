// P2pSource.h

#ifndef PPBOX_CDN_P2P_SOURCE_H_
#define PPBOX_CDN_P2P_SOURCE_H_

#include <ppbox/cdn/HttpStatistics.h>

#include <util/stream/url/HttpSource.h>

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
            : public  util::stream::HttpSource
        {
        public:
            P2pSource(
                boost::asio::io_service & io_svc);

            virtual ~P2pSource();

        public:
            virtual bool open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                boost::system::error_code & ec);

            virtual void async_open(
                framework::string::Url const & url,
                boost::uint64_t beg, 
                boost::uint64_t end, 
                response_type const & resp);

            virtual bool close(
                boost::system::error_code & ec);

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
                ppbox::demux::DemuxStatistic const & stat)
            {
            }

            virtual void parse_param(
                std::string const & params)
            {
            }

            virtual bool prepare(
                framework::string::Url & url, 
                boost::uint64_t & beg, 
                boost::uint64_t & end, 
                boost::system::error_code & ec)
            {
                ec.clear();
                return true;
            }

        private:
            ppbox::cdn::PptvMedia const * pptv_media_;
            ppbox::data::SegmentSource const * seg_source_;
            ppbox::cdn::HttpStatistics http_stat_;
        };

#ifdef PPBOX_DISABLE_PEER
        UTIL_REGISTER_URL_SOURCE("ppvod", P2pSource);
        UTIL_REGISTER_URL_SOURCE("ppvod2", P2pSource);
#endif

    } // namespace peer
} // namespace ppbox

#endif // PPBOX_PEER_PEER_SOURCE_H_
