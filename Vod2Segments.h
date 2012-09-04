//Vod2Segments.h

#ifndef PPBOX_CDN_VOD2_SEGMENTS_H_
#define PPBOX_CDN_VOD2_SEGMENTS_H_

#include "ppbox/cdn/Vod2Info.h"
#include "ppbox/cdn/PptvSegments.h"
#include "ppbox/cdn/HttpFetch.h"

#include <boost/asio/streambuf.hpp>

namespace util
{
    namespace protocol
    {
        class HttpClient;
    }
}

namespace framework
{
    namespace network
    {
        class NetName;
    }
}

namespace ppbox
{
    namespace cdn
    {

        class Vod2Segments
            : public PptvSegments
        {
        public:
            Vod2Segments(
                boost::asio::io_service & io_svc);

            ~Vod2Segments();

        public:
            void async_open(
                OpenMode mode,
                response_type const & resp);

            boost::system::error_code segment_url(
                size_t segment, 
                framework::string::Url & url,
                boost::system::error_code & ec);

            void cancel(
                boost::system::error_code & ec);

            void close(
                boost::system::error_code & ec);

            size_t segment_count() const;

            void segment_info(
                size_t segment, 
                ppbox::data::SegmentInfo & info) const;

            boost::system::error_code get_duration(
                ppbox::data::DurationInfo & info,
                boost::system::error_code & ec);

            virtual boost::system::error_code get_video(
                ppbox::data::VideoInfo & info,
                boost::system::error_code & ec);

            void set_url(
                framework::string::Url const &url);

        private:

            std::string Vod2Segments::get_key() const;

            framework::string::Url get_play_url();

            void handle_play(
                boost::system::error_code const & ec,
                boost::asio::streambuf & buf);

            void handle_async_open(
                boost::system::error_code const & ec);

            bool parse();

        private:
            struct StepType
            {
                enum Enum
                {
                    not_open, 
                    play,
                    finish
                };
            };

        private:
            VodPlayInfo vod_play_info_;

            std::string name_;
            framework::timer::Time local_time_;//用于计算key值
            StepType::Enum open_step_;
        };


    } // namespace cdn
} // namespace ppbox

#endif // PPBOX_CDN_VOD2_SEGMENTS_H_



