// TripMedia.h

#ifndef JUST_CDN_TRIP_TRIP_MEDIA_H_
#define JUST_CDN_TRIP_TRIP_MEDIA_H_

#include "just/cdn/p2p/P2pMedia.h"
#include "just/cdn/trip/TripMediaInfo.h"

namespace just
{
    namespace cdn
    {

        class TripMedia
            : public P2pMedia
        {
        public:
            TripMedia(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            ~TripMedia();

        public:
            virtual size_t segment_count() const;

            virtual bool segment_url(
                size_t segment, 
                framework::string::Url & url,
                boost::system::error_code & ec) const;

            virtual bool segment_info(
                size_t segment, 
                just::data::SegmentInfo & info,
                boost::system::error_code & ec) const;

        public:
            virtual void async_open(
                response_type const & resp);

            virtual void close(
                boost::system::error_code & ec);

        private:
            void parse_url(
                boost::system::error_code & ec);

            framework::string::Url & get_p2p_url(
                framework::string::Url & url);

            framework::string::Url & get_index_url(
                framework::string::Url & url);

            void handle_async_open(
                boost::system::error_code const & ec);

        private:
            struct StepType
            {
                enum Enum
                {
                    closed, 
                    p2p_meta,
                    cdn_index, 
                    finish
                };
            };

        private:
            StepType::Enum open_step_;
            ::trip::client::ResourceInfo info_;
            P2pVideo video_;
            P2pJump jump_;
        };

        JUST_REGISTER_MEDIA_BY_PROTOCOL("trip", TripMedia);

    } // namespace cdn
} // namespace just

#endif // JUST_CDN_TRIP_TRIP_MEDIA_H_
