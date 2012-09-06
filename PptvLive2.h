// PptvLive2.h

#ifndef _PPBOX_CDN_LIVE2_SEGMENTS_H_
#define _PPBOX_CDN_LIVE2_SEGMENTS_H_

#include "ppbox/cdn/PptvLive.h"
#include "ppbox/cdn/PptvLiveInfo2.h"

namespace ppbox
{
    namespace cdn
    {

        class PptvLive2
            : public PptvLive
        {
        public:
            PptvLive2(
                boost::asio::io_service & io_svc);

            ~PptvLive2();

        public:
            virtual void set_url(
                framework::string::Url const & url);

            virtual void async_open(
                response_type const & resp);

        private:
            void handle_async_open(
                boost::system::error_code const & ec);

            framework::string::Url get_jump_url() const;

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

        private:
            boost::uint32_t interval_;
            StepType::Enum open_step_;
            Live2JumpInfo jump_info_;
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_LIVE2_SEGMENTS_H_
