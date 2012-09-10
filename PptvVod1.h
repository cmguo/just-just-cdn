//PptvVod1.h

#ifndef _PPBOX_CDN_VOD1_SEGMENTS_H_
#define _PPBOX_CDN_VOD1_SEGMENTS_H_

#include "ppbox/cdn/PptvVodInfo1.h"
#include "ppbox/cdn/PptvVod.h"

namespace ppbox
{
    namespace cdn
    {

        class PptvVod1
            : public PptvVod
        {
        public:
            PptvVod1(
                boost::asio::io_service & io_svc);

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

            void handle_async_open(
                boost::system::error_code const & ec);

        private:
            struct StepType
            {
                enum Enum
                {
                    not_open, 
                    jump, 
                    drag,
                };
            };

        private:
            StepType::Enum open_step_;
            bool know_seg_count_;

            VodJumpInfo jump_info_;
            VodDragInfo drag_info_;

        };//VodSegmemt

        PPBOX_REGISTER_MEDIA(ppvod, PptvVod1);

    } // cdn
} // ppbox

#endif // _PPBOX_CDN_VOD1_SEGMENTS_H_
