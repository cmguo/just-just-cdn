// PptvLive1.h

#ifndef _JUST_CDN_PPTV_PPTV_LIVE1_H_
#define _JUST_CDN_PPTV_PPTV_LIVE1_H_

#include "just/cdn/pptv/PptvLiveInfo1.h"
#include "just/cdn/pptv/PptvLive.h"

namespace just
{
    namespace cdn
    {

        class PptvLive1
            : public PptvLive
        {
        public:
            PptvLive1(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            ~PptvLive1();

        public:
            virtual void async_open(
                response_type const & resp);

        public:
            virtual bool segment_url(
                size_t segment, 
                framework::string::Url & url, 
                boost::system::error_code & ec) const;

        private:
            void handle_async_open(
                boost::system::error_code ec);

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
            Live1JumpInfo jump_info_;
            std::string url_str_;
        };

        JUST_REGISTER_MEDIA_BY_PROTOCOL("pplive", PptvLive1);

    } // namespace cdn
} // namespace just

#endif // _JUST_CDN_PPTV_PPTV_LIVE1_H_
