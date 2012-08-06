//PptvSegments.h
#ifndef _PPBOX_CDN_PPTV_SEGMENT_H_
#define _PPBOX_CDN_PPTV_SEGMENT_H_

#include "ppbox/cdn/HttpFetch.h"
#include "ppbox/cdn/HttpStatistics.h"

#include <ppbox/common/SegmentBase.h>

#include <framework/timer/TickCounter.h>

namespace ppbox
{
    namespace cdn
    {

        class PptvSegments
            : public common::SegmentBase
        {
        public:
            PptvSegments(
                boost::asio::io_service & io_svc);

            ~PptvSegments();

            void set_url(
                framework::string::Url const &url);

            void cancel();

            void close();

        protected:
            void response(
                boost::system::error_code const & ec);

            void set_response(
                SegmentBase::response_type const & resp);

            HttpFetch & get_fetch()
            {
                return fetch_;
            }

            void open_logs_end(
                HttpStatistics const & http_stat, 
                int index, 
                boost::system::error_code const & ec);

        public:
            std::vector<HttpStatistics> const & open_logs() const
            {
                return open_logs_;
            }

            framework::string::Url const & get_url() const 
            {
                return cdn_url_;
            }

            boost::system::error_code const & last_error() const
            {
                return last_error_;
            }

            std::string server_host() const
            {
                return server_host_;
            }

            boost::uint32_t open_total_time() const
            {
                return open_total_time_;
            }

        protected:
            std::vector<HttpStatistics> open_logs_; // ²»³¬¹ý3¸ö
            framework::string::Url jdp_url_;//jump_drag_play_url
            framework::string::Url cdn_url_;

        protected:
            boost::system::error_code last_error_;
            std::string server_host_;
            boost::uint32_t open_total_time_;

        protected:
            framework::timer::TickCounter tc_;

        private:
            HttpFetch fetch_;
            SegmentBase::response_type resp_;

       
        };

    }//cdn
}//ppbox



#endif
