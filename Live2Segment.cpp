//Live2Segment.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/Live2Segment.h"

#include <util/protocol/pptv/Url.h>
#include <util/protocol/pptv/TimeKey.h>
#include <util/protocol/pptv/Base64.h>
#include <util/archive/XmlIArchive.h>
#include <util/archive/ArchiveBuffer.h>

#include <framework/string/Slice.h>
#include <framework/string/Parse.h>
#include <framework/string/Format.h>
using namespace framework::string;
#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Live2Segment", 0);

#ifndef PPBOX_DNS_LIVE2_JUMP
#  define PPBOX_DNS_LIVE2_JUMP "(tcp)(v4)live.dt.synacast.com:80"
#endif

namespace ppbox
{
    namespace cdn
    {
        static const  framework::network::NetName dns_live2_jump_server(PPBOX_DNS_LIVE2_JUMP);

        PPBOX_REGISTER_SEGMENT(pplive2, Live2Segment);

        static const boost::uint32_t CACHE_T = 1800;

        static inline std::string addr_host(
            framework::network::NetName const & addr)
        {
            return addr.host() + ":" + addr.svc();
        }

        Live2Segment::Live2Segment(
            boost::asio::io_service & io_svc)
            : PptvSegments(io_svc)
            , open_step_(StepType::not_open)
            , time_(0)
            , live_port_(0)
            , server_time_(0)
            , begin_time_(0)
            , value_time_(0)
            , seek_time_(0)
            , bwtype_(2)
            , index_(0)
            , interval_(5)
        {
            static boost::uint32_t g_seq = 1;
            seq_ += g_seq;
        }

        Live2Segment::~Live2Segment()
        {
        }

        void Live2Segment::async_open(
            OpenMode mode,
            response_type const & resp)
        {
            assert(StepType::not_open == open_step_);

            boost::system::error_code ec;
            if (name_.empty())
            {
                ec = error::bad_url;
            }

            mode_ = mode;
            set_response(resp);
            handle_async_open(ec);
        }

        void Live2Segment::handle_async_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                if (StepType::not_open == open_step_) {
                    LOG_S(Logger::kLevelAlarm, "parse url:failure");
                }
                if (ec != boost::asio::error::would_block) {
                    if (StepType::jump == open_step_) {
                        LOG_S(Logger::kLevelAlarm, "jump : failure"); 
                        open_logs_end(0, ec);
                        LOG_S(Logger::kLevelDebug, "jump failure (" << open_logs_[0].total_elapse << " milliseconds)");
                    }
                }
                last_error_ = ec;
                response(ec);
                return;
            }

            switch(open_step_) {
        case StepType::not_open:
            open_step_ = StepType::jump;
            LOG_S(Logger::kLevelEvent, "jump: start");
            async_fetch(
                get_jump_url(),
                dns_live2_jump_server,
                jump_info_, 
                boost::bind(&Live2Segment::handle_async_open, this ,_1));
            return;

        case StepType::finish:
            break;

        default:
            assert(0);
            return;
            }

            last_error_ = ec;

            response(ec);
            return;
        }


        void Live2Segment::set_info_by_jump(
            Live2JumpInfo & jump_info)
        {
            jump_info_ = jump_info;
            local_time_ = Time::now();
            server_time_ = jump_info_.server_time.to_time_t();

            begin_time_ = server_time_ - jump_info_.delay_play_time - CACHE_T;

            begin_time_ = begin_time_ / interval_ * interval_;
        }

        size_t Live2Segment::segment_index(
            boost::uint64_t time)
        {
            return time/1000/interval_;
        }

        framework::string::Url Live2Segment::get_jump_url() const
        {
            framework::string::Url url = jdp_url_;
            url.host(dns_live2_jump_server.host());
            url.svc(dns_live2_jump_server.svc());
            url.path("/live2/" + stream_id_);
            return url;
        }

        boost::system::error_code Live2Segment::reset(
            size_t & segment)
        {
            time_t file_time = server_time_ + (Time::now()-local_time_).total_seconds() - jump_info_.delay_play_time - seek_time_;
            file_time = file_time / interval_ * interval_;

            segment = (file_time - begin_time_) / interval_;

            segments_.add_segment(segment);

            return  boost::system::error_code();
        }

        size_t Live2Segment::segment_count() const
        {
            return size_t(-1);
        }

        void Live2Segment::segment_info(
            size_t segment, 
            common::SegmentInfo & info) const
        {
            if (segment < segments_.count()) {
                info.head_size = segments_[segment].head_leng;
                info.size = segments_[segment].file_size;
                info.time = segments_[segment].duration;
            } else {
                info.head_size = boost::uint64_t(-1);
                info.size = boost::uint64_t(-1);
                info.time = boost::uint64_t(-1);
            }
        }

        void Live2Segment::set_url(
            framework::string::Url const & url)
           {
            set_url(url);
            std::string strSeek = url.param("seek");
            if(!strSeek.empty()) {
                seek_time_ = framework::string::parse<boost::uint32_t>(strSeek);
            }
            std::string playlink = url.path().substr(1);
            // "[StreamID]-[Interval]-[datareate]
            if (playlink.find('-') != std::string::npos) {
                std::vector<std::string> strs;
                framework::string::slice<std::string>(playlink, std::inserter(strs, strs.end()), "-");
                if (strs.size() >= 3) {
                    name_ = "Stream:" + strs[0];
                    rid_.push_back(strs[0]);
                    stream_id_ = rid_[0];
                    framework::string::parse2(strs[1], interval_);
                    boost::uint32_t rate = 0;
                    framework::string::parse2(strs[2], rate);
                    rate_.push_back(rate);
                } else {
                    std::cout<<"Wrong URL Param"<<std::endl;
                }
                return;
            }
            std::string url_str = util::protocol::pptv::base64_decode(playlink, "kioe257ds");
            if (!url_str.empty()) {
                framework::string::map_find(url_str, "name", name_, "&");
                framework::string::map_find(url_str, "channel", channel_, "&");
                framework::string::map_find(url_str, "interval", interval_, "&");
                std::string sid, datarate;
                framework::string::map_find(url_str, "sid", sid, "&");
                framework::string::slice<std::string>(sid, std::inserter(rid_, rid_.end()), "@");
                if (!rid_.empty()) {
                    stream_id_ = rid_[0];
                }
                framework::string::map_find(url_str, "datarate", datarate, "&");
                framework::string::slice<boost::uint32_t>(datarate, std::inserter(rate_, rate_.end()), "@");
            }

        }

        boost::system::error_code Live2Segment::segment_url(
            size_t segment, 
            framework::string::Url & url,
            boost::system::error_code & ec)
        {
            ec.clear();
            time_t file_time = begin_time_ + (segment * interval_);
            url = url_; //这里使用原始传入的播放url
            url.host(jump_info_.server_host.host());
            url.svc(jump_info_.server_host.svc());
            url.path("/live/" + stream_id_ + "/" + format(file_time) + ".block");
            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] cdn url:" << url.to_string());

            url.param("replay", "1");
            url.param("source", "0");
//             url.param("name", name_);
            url.param("channel", channel_);
            url.param("interval", framework::string::format(interval_));
            url.param("start", framework::string::format(file_time));
            url.param("uniqueid", framework::string::format(seq_));
            framework::string::Url url_temp(url_.param("cdn.jump"));
            url.param("BWType", url_temp.param("bwtype"));

            std::string url_str = url.to_string();
            url_str += framework::string::join(rid_.begin(), rid_.end(), "@", "&rid=");
            url_str += framework::string::join(rate_.begin(), rate_.end(), "@", "&datarate=");

            url.from_string(url_str);

            return ec;
        }

        boost::system::error_code Live2Segment::get_duration(
            ppbox::common::DurationInfo & info,
            boost::system::error_code & ec)
        {
            ec.clear();
            info.total = 0;
            info.begin = 0;
            info.end = 0;
            info.redundancy = 0;
            info.interval = 0;

            return ec;
        }


    }//cdn
}//ppbox


// Live2Segment.cpp