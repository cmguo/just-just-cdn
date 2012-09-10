// PptvVod2.h

#ifndef PPBOX_CDN_PPTV_VOD2_H_
#define PPBOX_CDN_PPTV_VOD2_H_

#include "ppbox/cdn/PptvVodInfo2.h"
#include "ppbox/cdn/PptvVod.h"

namespace ppbox
{
    namespace cdn
    {

        class PptvVod2
            : public PptvVod
        {
        public:
            PptvVod2(
                boost::asio::io_service & io_svc);

            ~PptvVod2();

        public:
            void set_url(
                framework::string::Url const &url);

            void async_open(
                response_type const & resp);

        private:
            void parse_url(
                boost::system::error_code & ec);

            framework::string::Url & get_play_url(
                framework::string::Url & url);

            void handle_async_open(
                boost::system::error_code const & ec);

            void deside_ft();

        private:
            struct StepType
            {
                enum Enum
                {
                    not_open, 
                    play,
                    finish
                };
            };

        private:
            VodPlayInfo play_info_;
            StepType::Enum open_step_;
            size_t ft_;
        };

        PPBOX_REGISTER_MEDIA(ppvod2, PptvVod2);

    } // namespace cdn
} // namespace ppbox

#endif // PPBOX_CDN_PPTV_VOD2_H_
