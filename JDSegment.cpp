//JDSegment.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/JDSegment.h"

#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("JDSegment", 0);

namespace ppbox
{
    namespace cdn
    {
        JDSegment::JDSegment(boost::asio::io_service & ios)
            : VodSegments(ios)
        {

        }

        JDSegment::~JDSegment()
        {
        }

        boost::system::error_code JDSegment::get_request(
            size_t segment, 
            boost::uint64_t& beg, 
            boost::uint64_t& end, 
            std::string& url,
            boost::system::error_code & ec)
        {
            std::string url_t;
            ec = VodSegments::get_request(segment,beg,end, url_t, ec);
            url_t = framework::string::Url::encode(url_t);

            framework::string::Url url_new("http://127.0.0.1:9000/ppvaplaybyopen?" + segments_[segment].va_rid); 
            char const * headonly = (end <= segments_[segment].head_length )? "1" : "0";
            url_new.param("headonly", headonly);
            url_new.param("url", url_t);
            url_new.param("rid", segments_[segment].va_rid);
            url_new.param("blocksize", format(segments_[segment].block_size));
            url_new.param("filelength", format(segments_[segment].file_length));
            url_new.param("headlength", format(segments_[segment].head_length));

            url_new.param("autoclose", "false");
            url_new.param("drag", "1");
            url_new.param("BWType", format(bwtype_));
            url_new.param("blocknum", format(segments_[segment].block_num));

            url = url_new.to_string();

            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] vod url:"<<url);
            return ec;
        }

    }//cdn
}//ppbox


//JDSegment.cpp