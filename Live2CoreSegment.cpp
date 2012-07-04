//Live2CoreSegment.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/Live2CoreSegment.h"

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Live2CoreSegment", 0);
namespace ppbox
{
    namespace cdn
    {

        Live2CoreSegment::Live2CoreSegment(
            boost::asio::io_service & ios)
            : Live2Segment(ios)
        {

        }

        Live2CoreSegment::~Live2CoreSegment()
        {

        }

        boost::system::error_code Live2CoreSegment::get_request(
            size_t segment, 
            boost::uint64_t& beg, 
            boost::uint64_t& end, 
            std::string& url,
            boost::system::error_code & ec)
        {
            std::string url_t;
            ec = Live2Segment::get_request(segment,beg,end, url_t, ec);
            url_t = framework::string::Url::encode(url_t);

            std::string patcher("/playlive.flv?url=");
            patcher += url_t;
            patcher += "&channelid=";
            patcher += jump_info_.channelGUID;
            patcher += framework::string::join(rid_.begin(), rid_.end(), "@", "&rid=");
            patcher += framework::string::join(rate_.begin(), rate_.end(), "@", "&datarate=");
            patcher += "&replay=1";
            patcher += "&start=";
            patcher += framework::string::format(file_time_);
            patcher += "&interval=";
            patcher += framework::string::format(interval_);
            patcher += "&BWType=";
            patcher += framework::string::format(bwtype_);
            patcher += "&source=0&uniqueid=";
            patcher += framework::string::format(seq_);

            url = patcher;

            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] live2 url:"<<url);
            return ec;

        }

        boost::system::error_code Live2CoreSegment::get_duration(
            DurationInfo & info,
            boost::system::error_code & ec)
        {
            ec.clear();
            info.total = 0;
            info.begin = begin_time_ + (tc_.elapsed() / 1000);
            info.end = info.begin + value_time_;
            info.redundancy = jump_info_.delay_play_time;
            info.interval = interval_;

            return ec;
        }

    }//cdn
}//ppbox

//Live2CoreSegment.cpp
