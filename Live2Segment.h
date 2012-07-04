//Live2Segment.h

#ifndef _PPBOX_CDN_LIVE2_SEGMENT_H_
#define _PPBOX_CDN_LIVE2_SEGMENT_H_

#include "ppbox/cdn/SegmentBase.h"
#include "ppbox/cdn/LiveInfo.h"

#include <framework/timer/TickCounter.h>
#include <ppbox/common/HttpFetchManager.h>

namespace ppbox
{
    namespace cdn
    {
        struct Live2JumpInfo
        {
            framework::network::NetName server_host;
            std::vector<framework::network::NetName> server_hosts;
            util::serialization::UtcTime server_time;
            boost::uint16_t delay_play_time;
            std::string channelGUID;
            std::string server_limit;
            std::vector<std::string> server_limits;

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar & SERIALIZATION_NVP(server_host)
                    & SERIALIZATION_NVP(server_time)
                    & SERIALIZATION_NVP(delay_play_time)
                    & SERIALIZATION_NVP(server_limit)
                    & SERIALIZATION_NVP(channelGUID)
                    & SERIALIZATION_NVP(server_hosts)
                    & SERIALIZATION_NVP(server_limits);
            }
        };

        class Live2Segment
            : public SegmentBase
        {
        public:
            Live2Segment(
                boost::asio::io_service & io_svc);

            ~Live2Segment();

            virtual void async_open(
                response_type const & resp);

            virtual void set_reset_time(
                const char * url, 
                boost::uint32_t reset_play_time);

            void cancel(boost::system::error_code & ec) ;
            void close(boost::system::error_code & ec);
            bool is_open() ;

            size_t segment_count();

            bool next_segment(size_t segment, boost::uint32_t& out_time);

            size_t segment_index(boost::uint64_t time);

            void set_url(std::string const &url);

            boost::system::error_code get_request(
                size_t segment, 
                boost::uint64_t& beg, 
                boost::uint64_t& end, 
                std::string& url,
                boost::system::error_code & ec);

            boost::uint64_t segment_head_size(
                size_t segment);

            boost::uint64_t segment_body_size(
                size_t segment);

            boost::uint64_t segment_size(
                size_t segment);

            boost::uint32_t segment_time(
                size_t segment);

            boost::system::error_code get_duration(
                DurationInfo & info,
                boost::system::error_code & ec);

            void update_segment(size_t segment);

            void update_segment_file_size(size_t segment, boost::uint64_t fsize);

            void update_segment_duration(size_t segment, boost::uint32_t time);

            void update_segment_head_size(size_t segment, boost::uint64_t hsize);

            bool is_know_seg() const
            {
                return  false;
            }

        private:
            void handle_async_open(
                boost::system::error_code const & ec);

            void response(
                boost::system::error_code const & ec);

            framework::string::Url get_jump_url() const;

            void handle_jump(
                boost::system::error_code const & ec,
                boost::asio::streambuf & buf);

            void set_info_by_jump(
                Live2JumpInfo & jump_info);

            boost::system::error_code reset(
                size_t& segment);

            std::string get_key() const;


        private:
            struct StepType
            {
                enum Enum
                {
                    not_open, 
                    jump, 
                    finish, 
                };
            };

        protected:
            std::string name_;
            framework::network::NetName server_host_;
            LiveSegmentsInfo segments_;
            std::string key_;
            std::string url_;
            std::string channel_;
            std::string stream_id_;

            Live2JumpInfo jump_info_;
            Time local_time_;
            time_t server_time_;
            time_t file_time_;
            time_t begin_time_;
            time_t value_time_;
            framework::timer::TickCounter tc_;
            boost::uint32_t seek_time_;
            boost::int32_t bwtype_;
            boost::int32_t live_port_;
            int index_;
            std::vector<std::string> rid_;
            std::vector<boost::uint32_t> rate_;

            boost::uint16_t interval_;
            boost::uint32_t seq_;
            boost::uint32_t time_;


        private:
            ppbox::common::HttpFetchManager& fetch_mgr_;
            ppbox::common::FetchHandle handle_;
            StepType::Enum open_step_;
            SegmentBase::response_type resp_;



        };//Live2Segment

    }//cdn
}//ppbox

#endif//_PPBOX_CDN_LIVE2_SEGMENT_H_

// Live2Segment.h