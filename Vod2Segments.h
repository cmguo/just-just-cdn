//Vod2Segments.h

#ifndef PPBOX_CDN_VOD2_SEGMENTS_H_
#define PPBOX_CDN_VOD2_SEGMENTS_H_

#include "ppbox/cdn/VodInfo.h"

#include <ppbox/common/SegmentBase.h>
#include <ppbox/common/HttpFetchManager.h>

#include <ppbox/vod/HttpFetch.h>

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
            : public common::SegmentBase
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

            bool is_open();

            void cancel(
                boost::system::error_code & ec);

            void close(
                boost::system::error_code & ec);

            size_t segment_count();

            void segment_info(
                size_t segment, 
                common::SegmentInfo & info);

            boost::system::error_code get_duration(
                common::DurationInfo & info,
                boost::system::error_code & ec);

            void update_segment(size_t segment);

            void update_segment_file_size(size_t segment, boost::uint64_t fsize);

            void update_segment_duration(size_t segment, boost::uint32_t time);

            void update_segment_head_size(size_t segment, boost::uint64_t hsize);

            void set_url(std::string const &url);

            boost::system::error_code reset(size_t& segment);

            bool next_segment(
                size_t segment,
                boost::uint32_t &out_time){return true;}

            bool is_open(
                bool need_check_seek ,
                boost::system::error_code & ec);


        private:

            std::string Vod2Segments::get_key() const;

            framework::string::Url get_play_url();

            void handle_play(
                boost::system::error_code const & ec,
                boost::asio::streambuf & buf);

            void handle_async_open(
                boost::system::error_code const & ec);

            void response(
                boost::system::error_code const & ec);

            bool parse(const std::string name);


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
            ppbox::cdn::VodPlayInfo vod_play_info_;

            std::string name_;

            ppbox::common::HttpFetchManager& fetch_mgr_;
            ppbox::common::FetchHandle handle_;
            StepType::Enum open_step_;
            SegmentBase::response_type resp_;

            framework::network::NetName server_host_;
            boost::int32_t bwtype_;
            boost::int32_t ft_;
            std::string rid_;;
            std::string vvid_;
            std::string type_;
            std::string platform_;
            time_t server_time_;
            Time local_time_;
        };


    } // namespace cdn
} // namespace ppbox

#endif // PPBOX_CDN_VOD2_SEGMENTS_H_



