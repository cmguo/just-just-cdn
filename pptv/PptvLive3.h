// PptvLive3.h

#ifndef _PPBOX_CDN_PPTV_PPTV_LIVE3_H_
#define _PPBOX_CDN_PPTV_PPTV_LIVE3_H_

#include "ppbox/cdn/pptv/PptvLiveInfo3.h"
#include "ppbox/cdn/pptv/PptvLive.h"

namespace ppbox
{
    namespace cdn
    {

        class PptvLive3
            : public PptvLive
        {
        public:
            PptvLive3(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            ~PptvLive3();

        public:
            virtual void async_open(
                response_type const & resp);

        private:
            void parse_url(
                boost::system::error_code & ec);

            framework::string::Url & get_play_url(
                framework::string::Url & url) const;

            void handle_async_open(
                boost::system::error_code const & ec);

            void deside_ft(
                boost::system::error_code & ec);

        private:
            struct StepType
            {
                enum Enum
                {
                    closed, 
                    playing, 
                    finish, 
                };
            };

        private:
            StepType::Enum open_step_;
            LivePlayInfo play_info_;
            size_t ft_;
            bool noshift_;
        };

        PPBOX_REGISTER_MEDIA_BY_PROTOCOL("pplive3", PptvLive3);

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_PPTV_PPTV_LIVE3_H_
