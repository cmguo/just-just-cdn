//Live2Segment.h

#ifndef _PPBOX_CDN_LIVE2_SEGMENT_H_
#define _PPBOX_CDN_LIVE2_SEGMENT_H_

#include "ppbox/cdn/LiveInfo.h"
#include "ppbox/cdn/PptvSegments.h"
#include "ppbox/cdn/HttpFetch.h"

#include <ppbox/common/Serialize.h>

#include <util/serialization/stl/vector.h>

#include <framework/timer/TickCounter.h>


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
            : public PptvSegments
        {
        public:
            Live2Segment(
                boost::asio::io_service & io_svc);

            ~Live2Segment();

            virtual void async_open(
                OpenMode mode,
                response_type const & resp);

            void cancel(
                boost::system::error_code & ec) ;

            void close(
                boost::system::error_code & ec);

            bool is_open() ;

            size_t segment_count() const;

            bool next_segment(
                size_t segment, 
                boost::uint32_t& out_time);

            size_t segment_index(
                boost::uint64_t time);

            boost::system::error_code segment_url(
                size_t segment, 
                framework::string::Url& url,
                boost::system::error_code & ec);

            void segment_info(
                size_t segment, 
                common::SegmentInfo & info) const;

            boost::system::error_code get_duration(
                common::DurationInfo & info,
                boost::system::error_code & ec);

            void set_url(
                framework::string::Url const & url);

        public:
            std::string get_name()
            {
                return name_;
            }

        private:
            void handle_async_open(
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
            StepType::Enum open_step_;



        };//Live2Segment

    }//cdn
}//ppbox

#endif//_PPBOX_CDN_LIVE2_SEGMENT_H_

// Live2Segment.h