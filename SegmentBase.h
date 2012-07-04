//SegmentBase.h

#ifndef _PPBOX_CDN_SEGMENT_BASE_H_
#define _PPBOX_CDN_SEGMENT_BASE_H_

#include "ppbox/cdn/VodInfo.h"

#include <framework/string/Url.h>
#include <framework/network/NetName.h>

#include <util/protocol/http/HttpRequest.h>

#include <boost/asio/io_service.hpp>


namespace ppbox 
{
    namespace cdn 
    {
        struct DurationInfo
        {   
            DurationInfo()
                : redundancy(0)
                , begin(0)
                , end(0)
                , total(0)
            {
            }

            boost::uint32_t redundancy;
            boost::uint32_t begin;
            boost::uint32_t end;
            boost::uint32_t total;
            boost::uint32_t interval;
        }; 
        class SegmentBase
        {

        public:
            typedef boost::function<
                void(boost::system::error_code const &) > 
                response_type;

            SegmentBase(
                boost::asio::io_service & io_svc);

            ~SegmentBase();

            virtual std::string get_class_name()
            {
                return "SegmentBase";
            }

            virtual void async_open(
                response_type const & resp) = 0;

            virtual void set_reset_time(
                const char * url, 
                boost::uint32_t reset_play_time) = 0;

            virtual void cancel(boost::system::error_code & ec) = 0;
            virtual void close(boost::system::error_code & ec)= 0;
            virtual bool is_open() = 0;

            virtual size_t segment_count() =0;

            virtual bool next_segment(size_t segment, boost::uint32_t& out_time){return false;}
            
            virtual size_t segment_index(boost::uint64_t time){return 0;}

            virtual void set_url(std::string const &url);

            virtual boost::system::error_code reset(size_t& segment) = 0;

            virtual boost::system::error_code get_request(
                size_t segment, 
                boost::uint64_t& beg, 
                boost::uint64_t& end, 
                std::string& url,
                boost::system::error_code & ec) = 0;

            virtual boost::uint64_t segment_head_size(
                size_t segment) = 0;

            virtual boost::uint64_t segment_body_size(
                size_t segment) = 0;

            virtual boost::uint64_t segment_size(
                size_t segment) = 0;

            virtual boost::uint32_t segment_time(
                size_t segment) = 0;

            virtual boost::system::error_code get_duration(
                DurationInfo & info,
                boost::system::error_code & ec) = 0;

            virtual void update_segment(size_t segment) = 0;

            virtual void update_segment_file_size(size_t segment, boost::uint64_t fsize) = 0;

            virtual void update_segment_duration(size_t segment, boost::uint32_t time) = 0;

            virtual void update_segment_head_size(size_t segment, boost::uint64_t hsize) = 0;

            virtual bool is_know_seg() const = 0;

        protected:
            std::string url_;
            boost::asio::io_service &ios_service();

        private:
            boost::asio::io_service &ios_;
        };//SegmentBase

    }//cdn
}//ppbox


#endif//_PPBOX_CDN_SEGMENT_BASE_H_
