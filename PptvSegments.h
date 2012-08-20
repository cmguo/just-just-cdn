// PptvSegments.h

#ifndef _PPBOX_CDN_PPTV_SEGMENT_H_
#define _PPBOX_CDN_PPTV_SEGMENT_H_

#include "ppbox/cdn/HttpFetch.h"
#include "ppbox/cdn/HttpStatistics.h"

#include <ppbox/data/SegmentBase.h>

#include <framework/timer/TickCounter.h>

#include <boost/bind.hpp>
#include <boost/ref.hpp>

namespace ppbox
{
    namespace cdn
    {

        class PptvSegments
            : public ppbox::data::SegmentBase
        {
        public:
            PptvSegments(
                boost::asio::io_service & io_svc);

            ~PptvSegments();

            void set_url(
                framework::string::Url const &url);

            void cancel();

            void close();

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
            template <typename T>
            void async_fetch(
                framework::string::Url const & url, 
                framework::network::NetName const & server_host, 
                T & t, 
                HttpFetch::response_type const & resp);

            template <typename T>
            void handle_fetch(
                boost::system::error_code const & ec, 
                T & t, 
                HttpFetch::response_type const & resp);

            void response(
                boost::system::error_code const & ec);

            void set_response(
                SegmentBase::response_type const & resp);

            void open_logs_end(
                int index, 
                boost::system::error_code const & ec);

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

        template <typename T>
        void PptvSegments::async_fetch(
            framework::string::Url const & url, 
            framework::network::NetName const & server_host, 
            T & t, 
            HttpFetch::response_type const & resp)
        {
            fetch_.async_fetch(url, server_host, 
                boost::bind(&PptvSegments::handle_fetch<T>, this, _1, boost::ref(t), resp));
        }

        template <typename T>
        void PptvSegments::handle_fetch(
            boost::system::error_code const & ec, 
            T & t, 
            HttpFetch::response_type const & resp)
        {
            if (ec) {
                resp(ec);
                return;
            }

            util::archive::XmlIArchive<> ia(fetch_.data());
            ia >> t;
            if (!ia) {
                resp(error::bad_file_format);
                return;
            }
            resp(ec);
        }

    } // cdn
} // ppbox

#endif
