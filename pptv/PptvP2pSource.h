// PptvP2pSource.h

#ifndef JUST_CDN_PPTV_PPTV_P2P_SOURCE_H_
#define JUST_CDN_PPTV_PPTV_P2P_SOURCE_H_

#include <just/cdn/HttpStatistics.h>

#include <util/stream/url/HttpSource.h>

#include <util/event/Event.h>

#include <framework/string/Url.h>

namespace just
{
    namespace avbase
    {
        struct StreamStatus;
    }

    namespace data
    {
        class SegmentSource;
    }

    namespace cdn
    {

        class PptvMedia;

        class PptvP2pSource
            : public  util::stream::HttpSource
        {
        public:
            PptvP2pSource(
                boost::asio::io_service & io_svc);

            virtual ~PptvP2pSource();

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
            just::cdn::HttpStatistics const & http_stat() const;

            void pptv_media(
                PptvMedia const & media);

            just::cdn::PptvMedia const & pptv_media()
            {
                return *pptv_media_;
            }

        protected:
            void open_log(
                bool end);

            just::data::SegmentSource const & seg_source()
            {
                return *seg_source_;
            }

        private:
            void on_event(
                util::event::Observable const & sender, 
                util::event::Event const & event);

            virtual void on_stream_status(
                just::avbase::StreamStatus const & stat)
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
            just::cdn::PptvMedia const * pptv_media_;
            just::data::SegmentSource const * seg_source_;
            just::cdn::HttpStatistics http_stat_;
        };

#ifdef JUST_DISABLE_PEER
        UTIL_REGISTER_URL_SOURCE("ppvod", PptvP2pSource);
        UTIL_REGISTER_URL_SOURCE("ppvod2", PptvP2pSource);
#endif

    } // namespace cdn
} // namespace just

#endif // JUST_PEER_PEER_SOURCE_H_
