// P2pSource.h

#ifndef JUST_CDN_P2P_P2P_SOURCE_H_
#define JUST_CDN_P2P_P2P_SOURCE_H_

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

        class P2pMedia;

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
            just::cdn::HttpStatistics const & http_stat() const;

            void p2p_media(
                P2pMedia const & media);

            just::cdn::P2pMedia const & p2p_media()
            {
                return *p2p_media_;
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
            just::cdn::P2pMedia const * p2p_media_;
            just::data::SegmentSource const * seg_source_;
            just::cdn::HttpStatistics http_stat_;
        };

    } // namespace peer
} // namespace just

#endif // JUST_PEER_PEER_SOURCE_H_
