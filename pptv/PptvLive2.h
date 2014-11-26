// PptvLive2.h

#ifndef _PPBOX_CDN_PPTV_PPTV_LIVE2_H_
#define _PPBOX_CDN_PPTV_PPTV_LIVE2_H_

#include "ppbox/cdn/pptv/PptvLiveInfo2.h"
#include "ppbox/cdn/pptv/PptvLive.h"

namespace ppbox
{
    namespace cdn
    {

        class PptvLive2
            : public PptvLive
        {
        public:
            PptvLive2(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            ~PptvLive2();

        public:
            virtual void async_open(
                response_type const & resp);

        private:
            void handle_async_open(
                boost::system::error_code const & ec);

            framework::string::Url & get_jump_url(
                framework::string::Url & url) const;

        private:
            struct StepType
            {
                enum Enum
                {
                    closed, 
                    jumping, 
                    finish, 
                };
            };

        private:
            StepType::Enum open_step_;
            Live2JumpInfo jump_info_;
            LiveSegment seg_;
        };

        PPBOX_REGISTER_MEDIA_BY_PROTOCOL("pplive2", PptvLive2);

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_PPTV_PPTV_LIVE2_H_
