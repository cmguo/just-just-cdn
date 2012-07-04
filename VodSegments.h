//VodSegments.h

#ifndef _PPBOX_CDN_SEGMENT_VOD_H_
#define _PPBOX_CDN_SEGMENT_VOD_H_

#include "ppbox/cdn/SegmentBase.h"
#include "ppbox/cdn/VodInfo.h"
#include "ppbox/cdn/LiveInfo.h"

#include <ppbox/common/HttpFetchManager.h>


namespace ppbox
{
    namespace cdn
    {
        class VodSegments
            : public SegmentBase
        {
        public:
            VodSegments(
                boost::asio::io_service & io_svc);

            ~VodSegments();

            virtual std::string get_class_name()
            {
                return "VodSegments";
            }

            virtual void set_reset_time(
                const char * url, 
                boost::uint32_t reset_play_time)
            {
            }

            virtual void async_open(
                response_type const & resp);

            virtual boost::system::error_code get_request(
                size_t segment, 
                boost::uint64_t& beg, 
                boost::uint64_t& end, 
                std::string& url,
                boost::system::error_code & ec);

            virtual void cancel(boost::system::error_code & ec);
            virtual void close(boost::system::error_code & ec);
            virtual bool is_open();

            virtual framework::string::Url get_jump_url();
            virtual framework::string::Url get_drag_url();

            virtual size_t segment_count();

            virtual boost::uint64_t segment_head_size(
                size_t segment);

            virtual boost::uint64_t segment_body_size(
                size_t segment);

            virtual boost::uint64_t segment_size(
                size_t segment);

            virtual boost::uint32_t segment_time(
                size_t segment);

            virtual boost::system::error_code get_duration(
                DurationInfo & info,
                boost::system::error_code & ec);

            virtual void update_segment(size_t segment);

            virtual void update_segment_file_size(size_t segment, boost::uint64_t fsize);

            virtual void update_segment_duration(size_t segment, boost::uint32_t time);

            virtual void update_segment_head_size(size_t segment, boost::uint64_t hsize);

            virtual bool is_know_seg() const;

            virtual void set_url(std::string const &url);

            virtual boost::system::error_code reset(size_t& segment);
        private:

            std::string get_key() const;

            void handle_async_open(
                boost::system::error_code const & ec);

            void handle_jump(
                boost::system::error_code const & ec, 
                boost::asio::streambuf &buf);

            void handle_drag(
                boost::system::error_code const & ec, 
                boost::asio::streambuf &buf);

            void response(
                boost::system::error_code const & ec);

            void set_info_by_video(
                VodVideo & video);
            void add_segment(
                VodSegmentNew & segment);

        private:
            struct StepType
            {
                enum Enum
                {
                    not_open, 
                    jump, 
                    drag,
                    finish, 
                };
            };
        protected:
            std::string name_;//视频文件经过decode后的名字
            framework::network::NetName server_host_;
            boost::int32_t bwtype_;
            std::vector<VodSegmentNew> segments_;

        private:
            ppbox::common::HttpFetchManager& fetch_mgr_;
            ppbox::common::FetchHandle handle_;
            StepType::Enum open_step_;
            SegmentBase::response_type resp_;
            VodVideo * video_;
            bool know_seg_count_;

            framework::network::NetName proxy_addr_;

            time_t server_time_;
            Time local_time_;

        };//VodSegmemt

    }//cdn
}//ppbox


#endif//_PPBOX_CDN_VOD_SEGMENT_H_

//VodSegments.h