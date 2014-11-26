// PptvVod2.h

#ifndef PPBOX_CDN_PPTV_VOD2_H_
#define PPBOX_CDN_PPTV_VOD2_H_

#include "ppbox/cdn/pptv/PptvVodInfo2.h"
#include "ppbox/cdn/pptv/PptvVod.h"

namespace ppbox
{
    namespace cdn
    {

        class PptvVod2
            : public PptvVod
        {
        public:
            PptvVod2(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            ~PptvVod2();

        public:
            void async_open(
                response_type const & resp);

        private:
            void parse_url(
                boost::system::error_code & ec);

            framework::string::Url & get_play_url(
                framework::string::Url & url);

            void async_open2();

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
                    wait2, 
                    finish
                };
            };

        private:
            StepType::Enum open_step_;
            VodPlayInfo play_info_;
            size_t ft_;
        };

        PPBOX_REGISTER_MEDIA_BY_PROTOCOL("ppvod2", PptvVod2);

    } // namespace cdn
} // namespace ppbox

#endif // PPBOX_CDN_PPTV_VOD2_H_
