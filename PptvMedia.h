// PptvMedia.h

#ifndef _PPBOX_CDN_PPTV_SEGMENT_H_
#define _PPBOX_CDN_PPTV_SEGMENT_H_

#include "ppbox/cdn/PptvMediaInfo.h"
#include "ppbox/cdn/HttpFetch.h"
#include "ppbox/cdn/HttpStatistics.h"

#include <ppbox/data/MediaBase.h>

#include <util/event/Event.h>

#include <boost/shared_ptr.hpp>

namespace ppbox
{
    namespace demux
    {
        class BufferDemuxer;
    }

    namespace certify
    {
        class Certifier;
    }

    namespace dac
    {
        class DacModule;
    }

    namespace peer
    {
        class PeerSource;
    }

    namespace cdn
    {

        class PptvMedia
            : public ppbox::data::MediaBase
        {
        public:
            PptvMedia(
                boost::asio::io_service & io_svc);

            ~PptvMedia();

        public:
            virtual void set_url(
                framework::string::Url const &url);

            virtual void cancel();

            virtual void close();

        public:
            virtual boost::system::error_code get_info(
                ppbox::data::MediaInfo & info,
                boost::system::error_code & ec);

        public:
            Video const & video() const
            {
                return *video_;
            }

            Jump const & jump() const
            {
                return *jump_;
            }

            std::string const & user_host() const
            {
                return user_host_;
            }

            std::string const & p2p_params() const
            {
                return p2p_params_;
            }

            std::vector<HttpStatistics> const & open_logs() const
            {
                return open_logs_;
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

        protected:
            void response(
                boost::system::error_code const & ec);

            void set_response(
                MediaBase::response_type const & resp);

            bool is_demux() { return demuxer_ != NULL; }

            virtual void async_open2() {};

        protected:
            std::string get_key() const;

        protected:
            void set_video(
                Video & seg);

            void set_jump(
                Jump & seg);

            void set_user_host(
                std::string const & user_host);

        private:
            void response2(
                boost::system::error_code const & ec);

            void on_event(
                util::event::Event const & e);

        private:
            static bool parse_jump_param(
                Jump & jump, 
                std::string const & param);

            static bool parse_video_param(
                Video & video, 
                std::string const & param);

        protected:
            ppbox::certify::Certifier & cert_;
            ppbox::dac::DacModule & dac_;

            ppbox::demux::BufferDemuxer * demuxer_;

        protected:
            Video * video_;
            Jump * jump_;

            Jump parsed_jump_;
            Video parsed_video_;
            std::string user_host_;
            std::string p2p_params_;
            framework::timer::Time local_time_; // 用于计算key值

        protected:
            std::vector<HttpStatistics> open_logs_; // 不超过3个

        private:
            boost::shared_ptr<HttpFetch> fetch_;
            MediaBase::response_type resp_;
        };

        template <typename T>
        void PptvMedia::async_fetch(
            framework::string::Url const & url, 
            framework::network::NetName const & server_host, 
            T & t, 
            HttpFetch::response_type const & resp)
        {
            fetch_->async_fetch(url, server_host, 
                boost::bind(&PptvMedia::handle_fetch<T>, this, _1, boost::ref(t), resp));
        }

        template <typename T>
        void PptvMedia::handle_fetch(
            boost::system::error_code const & ec, 
            T & t, 
            HttpFetch::response_type const & resp)
        {
            open_logs_.push_back(fetch_->http_stat());

            if (ec) {
                resp(ec);
                return;
            }

            util::archive::XmlIArchive<> ia(fetch_->data());
            ia >> t;
            fetch_->close();
            if (!ia) {
                open_logs_.back().last_last_error = error::bad_file_format;
                resp(error::bad_file_format);
                return;
            }
            resp(ec);
        }

    } // cdn
} // ppbox

#endif
