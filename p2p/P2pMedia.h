// P2pMedia.h

#ifndef _JUST_CDN_P2P_P2P_MEDIA_H_
#define _JUST_CDN_P2P_P2P_MEDIA_H_

#include "just/cdn/p2p/P2pMediaInfo.h"
#include "just/cdn/HttpFetch.h"
#include "just/cdn/HttpStatistics.h"
#include "just/cdn/CdnError.h"

#include <just/data/segment/SegmentMedia.h>

#include <util/event/Event.h>
#include <util/archive/XmlIArchive.h>

#include <boost/shared_ptr.hpp>

namespace just
{
    namespace demux
    {
        class SegmentDemuxer;
    }

    namespace merge
    {
        class Merger;
    }

    namespace cdn
    {

        class P2pSource;

        class P2pMedia
            : public just::data::SegmentMedia
        {
        public:
            P2pMedia(
                boost::asio::io_service & io_svc,
                framework::string::Url const & url);

            ~P2pMedia();

        public:
            virtual void cancel(
                boost::system::error_code & ec);

            virtual void close(
                boost::system::error_code & ec);

        public:
            virtual bool get_basic_info(
                just::data::MediaBasicInfo & info,
                boost::system::error_code & ec) const;

            virtual bool get_info(
                just::data::MediaInfo & info,
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

            just::demux::SegmentDemuxer & demuxer() const
            {
                assert(owner_type_ == ot_demuxer);
                return *(owner_type_ == ot_demuxer ? (just::demux::SegmentDemuxer *)owner_ : NULL);
            }

            just::merge::Merger & merger() const
            {
                assert(owner_type_ == ot_merger);
                return *(owner_type_ == ot_merger ? (just::merge::Merger *)owner_ : NULL);
            }

        public:
            P2pVideo const & video() const
            {
                return *video_;
            }

            P2pJump const & jump() const
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
            void set_basic_info(
                just::data::MediaBasicInfo const & info);

            void set_video(
                P2pVideo & video);

            void set_jump(
                P2pJump & jump);

            void set_user_host(
                std::string const & user_host);

        private:
            void on_event(
                util::event::Observable const & sender, 
                util::event::Event const & event);

        private:
            void parse_url();

            static bool parse_jump_param(
                P2pJump & jump, 
                std::string const & param, 
                bool force = false);

            static bool parse_video_param(
                P2pVideo & video, 
                std::string const & param, 
                bool force = false);

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
            framework::string::Url url_;

            P2pVideo * video_;
            P2pJump * jump_;

        private:
            OnwerTypeEnum owner_type_;
            void * owner_;
            P2pSource * source_;

        private:
            MediaBase::response_type resp_;

            std::string p2p_params_;
            P2pJump parsed_jump_;
            P2pVideo parsed_video_;
            std::string user_host_;
            framework::timer::Time local_time_; // 用于计算key值

        private:
            boost::shared_ptr<HttpFetch> fetch_;
            std::vector<HttpStatistics> open_logs_; // 不超过3个
        };

        template <typename T>
        void P2pMedia::async_fetch(
            framework::string::Url const & url, 
            framework::network::NetName const & server_host, 
            T & t, 
            HttpFetch::response_type const & resp)
        {
            async_fetch(url, server_host, parse<T>, &t, resp);
        }

        template <typename T>
        void P2pMedia::parse(
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

    } // namespace cdn
} // namespace just

#endif // _JUST_CDN_P2P_P2P_MEDIA_H_
