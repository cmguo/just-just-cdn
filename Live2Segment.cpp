//Live2Segment.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/CdnError.h"
#include "ppbox/cdn/Live2Segment.h"

#include <ppbox/common/HttpFetchManager.h>

#include <util/protocol/pptv/Url.h>
#include <util/protocol/pptv/TimeKey.h>
#include <util/protocol/pptv/Base64.h>
#include <util/archive/XmlIArchive.h>
#include <util/archive/ArchiveBuffer.h>

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
            : SegmentBase(io_svc)
            , fetch_mgr_(util::daemon::use_module<ppbox::common::HttpFetchManager>(global_daemon()))
            , handle_(NULL)
            , open_step_(StepType::not_open)
            , time_(0)
            , live_port_(0)
            , server_time_(0)
            , file_time_(0)
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
            resp_ = resp;

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
                    }
                }
                response(ec);
                return;
            }

            switch(open_step_) {
        case StepType::not_open:
            open_step_ = StepType::jump;
            LOG_S(Logger::kLevelEvent, "jump: start");
            handle_ = fetch_mgr_.async_fetch(
                get_jump_url(),
                dns_live2_jump_server,
                boost::bind(&Live2Segment::handle_jump, this ,_1 , _2));
            return;

        case StepType::jump:
            open_step_ = StepType::finish;
            break;

        default:
            assert(0);
            return;
            }

            response(ec);
            return;
        }


        void Live2Segment::handle_jump(
            boost::system::error_code const & ec,
            boost::asio::streambuf & buf)
        {
            Live2JumpInfo  jump_info;
            boost::system::error_code ecc = ec;
            if (!ecc) {
                std::string buffer = boost::asio::buffer_cast<char const *>(buf.data());
                LOG_S(Logger::kLevelDebug2, "[handle_jump] jump buffer: " << buffer);

                util::archive::XmlIArchive<> ia(buf);
                ia >> jump_info;
                if (!ia) {
                    ecc = error::bad_file_format;
                } else {
                    set_info_by_jump(jump_info);
                }
            }
            handle_async_open(ecc);
        }

        void Live2Segment::set_info_by_jump(
            Live2JumpInfo & jump_info)
        {
            jump_info_ = jump_info;
            local_time_ = Time::now();
            server_time_ = jump_info_.server_time.to_time_t();

            file_time_ = server_time_ - jump_info_.delay_play_time;
            file_time_ = file_time_ / interval_ * interval_;

            begin_time_ = file_time_ - CACHE_T;
            file_time_ = begin_time_;
        }

        size_t Live2Segment::segment_index(
            boost::uint64_t time)
        {
            return time/1000/interval_;
        }

        framework::string::Url Live2Segment::get_jump_url() const
        {
            framework::string::Url url("http://localhost/");
            url.host(dns_live2_jump_server.host());
            url.svc(dns_live2_jump_server.svc());
            url.path("/live2/" + stream_id_);
            return url;
        }

        boost::system::error_code Live2Segment::reset(
            size_t& segment)
        {
            file_time_ = server_time_ + (Time::now()-local_time_).total_seconds() - jump_info_.delay_play_time;
            file_time_ = file_time_ / interval_ * interval_;

            segment = (file_time_ - begin_time_) / interval_;

            segments_.add_segment(segment);

            return  boost::system::error_code();
        }


        void Live2Segment::cancel(
            boost::system::error_code & ec) 
        {
            fetch_mgr_.cancel(handle_);
        }

        void Live2Segment::close(
            boost::system::error_code & ec)
        {
            fetch_mgr_.close(handle_);
        }

        bool Live2Segment::is_open()
        {
            switch (open_step_) {
        case StepType::jump:
        case StepType::finish:
            return true;
        default:
            return false;
            }
        }

        bool Live2Segment::next_segment(
            size_t segment, 
            boost::uint32_t& out_time)
        {
            size_t segment_count = CACHE_T / interval_;
            boost::uint32_t iDownload = ((segment > segment_count)?(segment - segment_count):0)*interval_;
            boost::uint32_t iSystem = (Time::now()-local_time_).total_seconds();
            if (iDownload > iSystem)
            {
                out_time = iDownload - iSystem;
            }
            else
            {
                out_time = 0;
            }
            return true;
        }

        size_t Live2Segment::segment_count() 
        {
            return size_t(-1);
        }

        void Live2Segment::segment_info(
            size_t segment, 
            common::SegmentInfo & info)
        {
            if (segment < segments_.size()) {
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
            std::string const &url)
        {

            std::string::size_type slash = url.find('|');
            if (slash == std::string::npos) {
                return;
            }

            key_ = url.substr(0, slash);
            std::string playlink = url.substr(slash + 1);

            playlink = framework::string::Url::decode(playlink);
            framework::string::Url request_url(playlink);
            playlink = request_url.path().substr(1);

            std::string strSeek = request_url.param("seek");
            if(!strSeek.empty())
            {
                seek_time_ = framework::string::parse<boost::uint32_t>(strSeek);
            }
            std::string strBWType = request_url.param("bwtype");
            if(!strBWType.empty())
            {
                bwtype_ = framework::string::parse<boost::int32_t>(strBWType);
            }

            if (playlink.find('-') != std::string::npos) {
                // "[StreamID]-[Interval]-[datareate]
                url_ = playlink;
                std::vector<std::string> strs;
                slice<std::string>(playlink, std::inserter(strs, strs.end()), "-");
                if (strs.size() >= 3) {
                    name_ = "Stream:" + strs[0];
                    rid_.push_back(strs[0]);
                    stream_id_ = rid_[0];
                    parse2(strs[1], interval_);
                    boost::uint32_t rate = 0;
                    parse2(strs[2], rate);
                    rate_.push_back(rate);
                } else {
                    std::cout<<"Wrong URL Param"<<std::endl;
                }
                return;
            }
            url_ = util::protocol::pptv::base64_decode(playlink, key_);
            if (!url_.empty()) {
                map_find(url_, "name", name_, "&");
                map_find(url_, "channel", channel_, "&");
                map_find(url_, "interval", interval_, "&");
                std::string sid, datarate;
                map_find(url_, "sid", sid, "&");
                slice<std::string>(sid, std::inserter(rid_, rid_.end()), "@");
                if (!rid_.empty())
                    stream_id_ = rid_[0];
                map_find(url_, "datarate", datarate, "&");
                slice<boost::uint32_t>(datarate, std::inserter(rate_, rate_.end()), "@");
            }

        }

        boost::system::error_code Live2Segment::segment_url(
            size_t segment, 
            framework::string::Url& url,
            boost::system::error_code & ec)
        {
            ec.clear();

            file_time_ = begin_time_ + (segment * interval_);

            std::string url_str ="http://" + jump_info_.server_host.host_svc() + "/live/" + stream_id_ + "/" + format(file_time_) + ".block";
            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] live2 cdn url:" << url_str);

            url_str += "&channelid=";
            url_str += jump_info_.channelGUID;
            url_str += framework::string::join(rid_.begin(), rid_.end(), "@", "&rid=");
            url_str += framework::string::join(rate_.begin(), rate_.end(), "@", "&datarate=");
            url_str += "&replay=1";
            url_str += "&start=";
            url_str += framework::string::format(file_time_);
            url_str += "&interval=";
            url_str += framework::string::format(interval_);
            url_str += "&BWType=";
            url_str += framework::string::format(bwtype_);
            url_str += "&SourceBase=0&uniqueid=";
            url_str += framework::string::format(seq_);

            url.from_string(url_str);
            return ec;
        }

        void Live2Segment::response(
            boost::system::error_code const & ec)
        {
            SegmentBase::response_type resp;
            resp.swap(resp_);
            resp(ec);
        }

        boost::system::error_code Live2Segment::get_duration(
            common::DurationInfo & info,
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

        void Live2Segment::update_segment(size_t segment)
        {
            segments_.add_segment(segment);
        }

        void Live2Segment::update_segment_file_size(size_t segment, boost::uint64_t fsize)
        {
            segments_[segment].file_size = fsize;
        }

        void Live2Segment::update_segment_duration(size_t segment, boost::uint32_t time)
        {
            segments_[segment].duration = 5;
        }

        void Live2Segment::update_segment_head_size(size_t segment, boost::uint64_t hsize)
        {
            segments_[segment].head_leng = hsize;
        }

    }//cdn
}//ppbox


// Live2Segment.cpp