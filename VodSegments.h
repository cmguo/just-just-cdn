//VodSegments.h

#ifndef _PPBOX_CDN_SEGMENT_VOD_H_
#define _PPBOX_CDN_SEGMENT_VOD_H_

#include "ppbox/cdn/VodInfo.h"
#include "ppbox/cdn/LiveInfo.h"

#include <ppbox/common/SegmentBase.h>
#include <ppbox/common/HttpFetchManager.h>

#include <util/protocol/pptv/Url.h>

namespace ppbox
{
    namespace cdn
    {
        class VodSegments
            : public common::SegmentBase
        {
        public:
            VodSegments(
                boost::asio::io_service & io_svc);

            ~VodSegments();

            virtual void async_open(
                OpenMode mode,
                response_type const & resp);

            virtual boost::system::error_code segment_url(
                size_t segment, 
                framework::string::Url & url,
                boost::system::error_code & ec);

            virtual void cancel(
                boost::system::error_code & ec);

            virtual void close(
                boost::system::error_code & ec);

            virtual bool is_open();


            virtual size_t segment_count();

            virtual void segment_info(
                size_t segment, 
                common::SegmentInfo & info);

            virtual boost::system::error_code get_duration(
                common::DurationInfo & info,
                boost::system::error_code & ec);

            virtual void update_segment(
                size_t segment);

            virtual void update_segment_file_size(
                size_t segment, boost::uint64_t fsize);

            virtual void update_segment_duration(
                size_t segment,
                boost::uint32_t time);

            virtual void update_segment_head_size(
                size_t segment, 
                boost::uint64_t hsize);

            virtual void set_url(
                std::string const &url);

            bool next_segment(
                size_t segment,
                boost::uint32_t & out_time){return true;}

            virtual boost::system::error_code reset(
                size_t& segment);

        private:
            framework::string::Url get_jump_url();
            framework::string::Url get_drag_url();

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
            VodJumpInfo jump_info_;
            VodDragInfo drag_info_;
            std::string name_;

        private:
            ppbox::common::HttpFetchManager& fetch_mgr_;
            ppbox::common::FetchHandle handle_;
            StepType::Enum open_step_;
            SegmentBase::response_type resp_;
            bool know_seg_count_;
            time_t server_time_;//用于计算key值
            Time local_time_;

        };//VodSegmemt

    }//cdn
}//ppbox


#endif//_PPBOX_CDN_VOD_SEGMENT_H_

//VodSegments.h