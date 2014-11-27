// PptvVod1.h

#ifndef _JUST_CDN_PPTV_PPTV_VOD1_H_
#define _JUST_CDN_PPTV_PPTV_VOD1_H_

#include "just/cdn/pptv/PptvVodInfo1.h"
#include "just/cdn/pptv/PptvVod.h"

namespace just
{
    namespace cdn
    {

        class PptvVod1
            : public PptvVod
        {
        public:
            PptvVod1(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            ~PptvVod1();

        public:
            virtual void async_open(
                response_type const & resp);

        private:
            void parse_url(
                framework::string::Url const & url);

            framework::string::Url & get_jump_url(
                framework::string::Url & url);

            framework::string::Url & get_drag_url(
                framework::string::Url & url);

            virtual void async_open2();

            void handle_async_open(
                boost::system::error_code const & ec);

        private:
            struct StepType
            {
                enum Enum
                {
                    closed, 
                    jumping, 
                    wait2,
                    draging,
                    finish, 
                };
            };

        private:
            StepType::Enum open_step_;
            bool know_seg_count_;

            VodJumpInfo jump_info_;
            VodDragInfo drag_info_;

        };//VodSegmemt

        JUST_REGISTER_MEDIA_BY_PROTOCOL("ppvod", PptvVod1);

    } // namespace cdn
} // namespace just

#endif // _JUST_CDN_PPTV_PPTV_VOD1_H_
