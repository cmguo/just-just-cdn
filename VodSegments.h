//VodSegments.h

#ifndef _PPBOX_CDN_SEGMENT_VOD_H_
#define _PPBOX_CDN_SEGMENT_VOD_H_

#include "ppbox/cdn/VodInfo.h"
#include "ppbox/cdn/PptvSegments.h"

namespace ppbox
{
    namespace cdn
    {

        class VodSegments
            : public PptvSegments
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

            virtual size_t segment_count() const;

            virtual void segment_info(
                size_t segment, 
                ppbox::data::SegmentInfo & info) const;

            virtual boost::system::error_code get_duration(
                ppbox::data::DurationInfo & info,
                boost::system::error_code & ec);

           virtual void set_url(
                framework::string::Url const &url);

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

        private:
            StepType::Enum open_step_;
            OpenMode mode_;
            bool know_seg_count_;
            Time local_time_;//用于计算key值

        };//VodSegmemt

    }//cdn
}//ppbox


#endif//_PPBOX_CDN_VOD_SEGMENT_H_

//VodSegments.h