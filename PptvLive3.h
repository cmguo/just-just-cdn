// PptvLive3.h

#ifndef _PPBOX_CDN_PPTV_LIVE3_H_
#define _PPBOX_CDN_PPTV_LIVE3_H_

#include "ppbox/cdn/PptvLive.h"
#include "ppbox/cdn/PptvLiveInfo3.h"

namespace ppbox
{
    namespace cdn
    {

        class PptvLive3
            : public PptvLive
        {
        public:
            PptvLive3(
                boost::asio::io_service & io_svc);

            ~PptvLive3();

        public:
            virtual void async_open(
                response_type const & resp);

        private:
            void handle_async_open(
                boost::system::error_code const & ec);

            framework::string::Url & get_play_url(
                framework::string::Url & url) const;

        private:
            struct StepType
            {
                enum Enum
                {
                    not_open, 
                    playing, 
                    finish, 
                };
            };

        private:
            StepType::Enum open_step_;
            LivePlayInfo play_info_;
        };

        PPBOX_REGISTER_MEDIA(pplive3, PptvLive3);

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_PPTV_LIVE3_H_
