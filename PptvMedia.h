// PptvMedia.h

#ifndef _PPBOX_CDN_PPTV_SEGMENT_H_
#define _PPBOX_CDN_PPTV_SEGMENT_H_

#include "ppbox/cdn/PptvMediaInfo.h"
#include "ppbox/cdn/HttpFetch.h"
#include "ppbox/cdn/HttpStatistics.h"
#include "ppbox/cdn/CdnError.h"

#include <ppbox/data/SegmentMedia.h>

#include <util/event/Event.h>
#include <util/archive/XmlIArchive.h>

#include <boost/shared_ptr.hpp>

namespace ppbox
{
    namespace demux
    {
        class SegmentDemuxer;
    }

    namespace merge
    {
        class MergerBase;
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
            : public ppbox::data::SegmentMedia
        {
        public:
            PptvMedia(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            ~PptvMedia();

        public:
            virtual void cancel(
                boost::system::error_code & ec);

            virtual void close(
                boost::system::error_code & ec);

        public:
            virtual bool get_info(
                ppbox::data::MediaInfo & info,
                boost::system::error_code & ec) const;

            virtual bool get_url(
                framework::string::Url & url,
                boost::system::error_code & ec) const;

        public:
            enum OnwerTypeEnum
            {
                ot_none, 
                ot_demuxer, 
                ot_merger, 
            };

        public:
            OnwerTypeEnum owner_type() const { return owner_type_; }

            ppbox::demux::SegmentDemuxer & demuxer() const
            {
                assert(owner_type_ == ot_demuxer);
                return *(owner_type_ == ot_demuxer ? (ppbox::demux::SegmentDemuxer *)owner_ : NULL);
            }

            ppbox::merge::MergerBase & merger() const
            {
                assert(owner_type_ == ot_merger);
                return *(owner_type_ == ot_merger ? (ppbox::merge::MergerBase *)owner_ : NULL);
            }

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
            typedef void (* parser_t)(
                HttpFetch & fetch, 
                void * t, 
                boost::system::error_code & ec);

            template <typename T>
            void async_fetch(
                framework::string::Url const & url, 
                framework::network::NetName const & server_host, 
                T & t, 
                HttpFetch::response_type const & resp);

        protected:
            void response(
                boost::system::error_code const & ec);

            void set_response(
                MediaBase::response_type const & resp);

            virtual void async_open2() {};

        protected:
            std::string get_key() const;

        protected:
            void set_video(
                Video & video);

            void set_jump(
                Jump & jump);

            void set_user_host(
                std::string const & user_host);

        private:
            void response2(
                boost::system::error_code const & ec);

            void on_event(
                util::event::Event const & e);

        private:
            void parse_url();

            static bool parse_jump_param(
                Jump & jump, 
                std::string const & param);

            static bool parse_video_param(
                Video & video, 
                std::string const & param);

            template <typename T>
            static void parse(
                HttpFetch & fetch, 
                void * t, 
                boost::system::error_code & ec);

            void async_fetch(
                framework::string::Url const & url, 
                framework::network::NetName const & server_host, 
                parser_t parser, 
                void * t, 
                HttpFetch::response_type const & resp);

            void handle_fetch(
                boost::system::error_code const & ec, 
                parser_t parser, 
                void * t, 
                HttpFetch::response_type const & resp);

        protected:
            ppbox::certify::Certifier & cert_;
            ppbox::dac::DacModule & dac_;

        protected:
            framework::string::Url url_;

            Video * video_;
            Jump * jump_;

        private:
            OnwerTypeEnum owner_type_;
            void * owner_;

        private:
            MediaBase::response_type resp_;

            std::string p2p_params_;
            Jump parsed_jump_;
            Video parsed_video_;
            std::string user_host_;
            framework::timer::Time local_time_; // 用于计算key值

        private:
            boost::shared_ptr<HttpFetch> fetch_;
            std::vector<HttpStatistics> open_logs_; // 不超过3个
        };

        template <typename T>
        void PptvMedia::async_fetch(
            framework::string::Url const & url, 
            framework::network::NetName const & server_host, 
            T & t, 
            HttpFetch::response_type const & resp)
        {
            async_fetch(url, server_host, parse<T>, &t, resp);
        }

        template <typename T>
        void PptvMedia::parse(
            HttpFetch & fetch, 
            void * t, 
            boost::system::error_code & ec)
        {
            util::archive::XmlIArchive<> ia(fetch.data());
            ia >> *(T *)t;
            if (ia) {
                ec.clear();
            } else {
                ec = error::bad_file_format;
            }
        }

    } // cdn
} // ppbox

#endif
