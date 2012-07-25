//Vod2Segments.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/Vod2Segments.h"
#include "ppbox/cdn/CdnError.h"

#include <ppbox/common/HttpFetchManager.h>

#include <util/protocol/pptv/Url.h>
#include <util/protocol/pptv/TimeKey.h>
#include <util/archive/XmlIArchive.h>
#include <util/archive/ArchiveBuffer.h>
#include <util/buffers/BufferCopy.h>

#include <framework/string/Parse.h>
#include <framework/logger/LoggerStreamRecord.h>
using namespace framework::logger;

FRAMEWORK_LOGGER_DECLARE_MODULE_LEVEL("Vod2Segment", 0);

#ifndef PPBOX_DNS_VOD_PLAY
#define PPBOX_DNS_VOD_PLAY "(tcp)(v4)epg.api.pptv.com:80"
#endif

#ifndef JUMP_TYPE
#define JUMP_TYPE "ppbox"
#endif

#ifndef PPBOX_VOD_PLATFORM
#define PPBOX_VOD_PLATFORM "ppbox"
#endif

#ifndef PPBOX_VOD_TYPE
#define PPBOX_VOD_TYPE "ppbox"
#endif

namespace ppbox
{
    namespace cdn
    {

        static const framework::network::NetName dns_vod_play_server(PPBOX_DNS_VOD_PLAY);

        PPBOX_REGISTER_SEGMENT(ppvod2, Vod2Segments);

        Vod2Segments::Vod2Segments(
            boost::asio::io_service & io_svc)
            : SegmentBase(io_svc)
            , fetch_mgr_(util::daemon::use_module<ppbox::common::HttpFetchManager>(global_daemon()))
            , handle_(NULL)
            , open_step_(StepType::not_open)
            , ft_(-1)
            , bwtype_(-1)
        {
        }

        Vod2Segments::~Vod2Segments()
        {
        }

        void Vod2Segments::async_open(
            OpenMode mode,
            response_type const & resp)
        {
            assert(StepType::not_open == open_step_);
            boost::system::error_code ec;
            resp_ = resp;

            if (parse(name_)) {
                if (OpenMode::fast == mode) {
                    response(ec);//及时回调
                    return;
                } else { //OpenMode::slow == mode
                    if(vod_play_info_.video.duration == vod_play_info_.segment.segments[0].duration) {
                        response(ec);
                        vod_play_info_.is_ready = true;
                        return;
                    } else {
                        open_step_ = StepType::play;
                    }
                } 
            } else {
                open_step_ = StepType::play;
            }
                handle_async_open(ec);
        }

        void Vod2Segments::handle_async_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                LOG_S(Logger::kLevelAlarm, "play: failure");
                response(ec);
                return;
            }

            switch (open_step_) {
            case StepType::play:
                {
                    LOG_S(Logger::kLevelEvent, "play: start");
                    fetch_mgr_.async_fetch(
                        get_play_url(),
                        dns_vod_play_server,
                        boost::bind(&Vod2Segments::handle_play, this, _1, _2));
                    return;
                }
            case StepType::finish:
                {
                   break;
                }
            default:
                return;
            }

            response(ec);
        }

        void Vod2Segments::handle_play(
            boost::system::error_code const & ec, 
            boost::asio::streambuf & buf)
        {
            if (ec) {
                vod_play_info_.ec = ec;
                vod_play_info_.is_ready = false;
                LOG_S(Logger::kLevelAlarm, "play: failure");
                handle_async_open(ec);
                return;
            }

            std::string buffer = boost::asio::buffer_cast<char const *>(buf.data());
            LOG_STR(Logger::kLevelDebug2, buffer.c_str());

            boost::asio::streambuf buf2;
            util::buffers::buffer_copy(buf2.prepare(buf.size()), buf.data());
            buf2.commit(buf.size());

            util::archive::XmlIArchive<> ia(buf2);
            ia >> vod_play_info_;
            if (!ia) {
                util::archive::XmlIArchive<> ia2(buf);
                ppbox::cdn::VodPlayInfoDrag play_info_old;
                ia2 >>play_info_old;
                if(!ia2) {
                    vod_play_info_.ec = error::bad_file_format;
                    LOG_S(Logger::kLevelError, "parse play failed");
                    handle_async_open(ec);
                } else {
                    vod_play_info_ = play_info_old;
                }
            }
            else {
                LOG_S(Logger::kLevelEvent, "play: success");
            }


            boost::int32_t max_bitrate = 0;
            std::vector<Vod2Video>::iterator temp_ter, iter = vod_play_info_.channel.file.begin();
            while (iter != vod_play_info_.channel.file.end()) {
                if (iter->bitrate > max_bitrate) {
                    max_bitrate = iter->bitrate;
                    temp_ter = iter;
                }
                iter++;
            }
            vod_play_info_.video.ft = temp_ter->ft;
            vod_play_info_.video.rid = temp_ter->rid;
            vod_play_info_.video.bitrate = temp_ter->bitrate;
            vod_play_info_.video.filesize = temp_ter->filesize;
            vod_play_info_.video.duration = temp_ter->duration;
            vod_play_info_.video.width = temp_ter->width;
            vod_play_info_.video.height = temp_ter->height;



            std::vector<Vod2SegmentNew>::iterator itr = vod_play_info_.drag.begin();
            while(itr != vod_play_info_.drag.end()) {
                if (itr->ft == temp_ter->ft) {
                    vod_play_info_.ft = itr->ft;
                    vod_play_info_.segment.ft = itr->ft;
                    vod_play_info_.segment.segments = itr->segments;
                    break;
                } else {
                    itr ++;
                }
            }

            std::vector<Vod2DtInfo>::iterator it = vod_play_info_.dt.begin();
            while (it != vod_play_info_.dt.end()) 
            {
                if (it->ft == temp_ter->ft) {
                    vod_play_info_.dtinfo.ft = it->ft;
                    vod_play_info_.dtinfo.bwt = it ->bwt;
                    vod_play_info_.dtinfo.id = it ->id;
                    vod_play_info_.dtinfo.sh = it ->sh;
                    vod_play_info_.dtinfo.bh = it ->bh;
                    vod_play_info_.dtinfo.st = it ->st;
                    break;
                } else {
                    it++;
                }
            }
            vod_play_info_.is_ready = true;

            ft_ = vod_play_info_.video.ft;
            bwtype_ = vod_play_info_.dtinfo.bwt;
            server_host_ = vod_play_info_.dtinfo.sh;
            rid_ = vod_play_info_.dtinfo.id;

            open_step_ = StepType::finish;
            handle_async_open(ec);
        }


        void Vod2Segments::cancel(boost::system::error_code & ec)
        {
            fetch_mgr_.cancel(handle_);
        }

        void Vod2Segments::close(boost::system::error_code & ec)
        {
            fetch_mgr_.close(handle_);
        }

        bool Vod2Segments::is_open()
        {
            switch (open_step_) 
            {
            case StepType::finish:
                break;
            case StepType::play:
                {
                    if (vod_play_info_.is_ready) {
                        open_step_ = StepType::finish;
                        break;
                    } 
                }
            default:
                return false;
            }

            return true;
        }

        bool Vod2Segments::parse(const std::string name)
        {//验证串第一段信息是否完整，完整先播第一段，否则就先请求play
            framework::string::Url request_url(name_);
            //name
            std::string tmp_param = request_url.param("f");
            if(tmp_param.empty())
            {
                tmp_param = request_url.param("ft");
                if(tmp_param.empty())
                {
                    LOG_S(Logger::kLevelDebug, "parse ft or f failed");
                    return false;
                }
            }
            parse2(tmp_param.c_str(), vod_play_info_.ft);
            ft_ = vod_play_info_.ft;

            //总时长
            tmp_param = request_url.param("duration");
            if(tmp_param.empty())
            {
                LOG_S(Logger::kLevelDebug, "parse duration failed");
                return false;
            }
            float fTotalDur = 0.0;
            parse2(tmp_param.c_str(), fTotalDur);
            vod_play_info_.video.duration = (boost::uint32_t)(fTotalDur*1000.0f);

            //rid
            tmp_param = request_url.param("name");
            if(tmp_param.empty())
            {
                LOG_S(Logger::kLevelDebug, "parse name failed");
                return false;
            }
            vod_play_info_.video.rid = tmp_param;
            rid_ = tmp_param;

            //server host
            tmp_param = request_url.param("svrhost");
            if(tmp_param.empty())
            {
                LOG_S(Logger::kLevelDebug, "parse svrhost failed");
                return false;
            }
            framework::network::NetName svrhost(tmp_param.c_str());
            server_host_ = svrhost;

            //server time
            tmp_param = request_url.param("svrtime");
            if(tmp_param.empty())
            {
                LOG_S(Logger::kLevelDebug, "parse svrtime failed");
                return false;
            }
            boost::uint64_t time_utc = 0;
            parse2(tmp_param.c_str(), time_utc);
            server_time_ = time_utc;

            //bwtype
            tmp_param = request_url.param("bwtype");
            if(tmp_param.empty())
            {
                LOG_S(Logger::kLevelDebug, "parse bwtype failed");
                return false;
            }
            boost::int32_t bwtype= 0;
            parse2(tmp_param.c_str(), bwtype);
            bwtype_ = bwtype;

            //drag
            tmp_param = request_url.param("drag");
            if(tmp_param.empty())
            {
                LOG_S(Logger::kLevelDebug, "parse drag failed");
                return false;
            }
            std::vector<std::string> strs;
            slice<std::string>(tmp_param, std::inserter(strs, strs.end()), "|");
            if(strs.size() != 3)
            {
                LOG_S(Logger::kLevelDebug, "parse drag info failed");
                return false;
            }

            boost::uint32_t duration = 0;
            boost::uint64_t head_length = 0;
            boost::uint64_t file_length = 0;
            float fDur = 0.0;

            parse2(strs[0], head_length);
            parse2(strs[1], file_length);
            parse2(strs[2], fDur);
            duration = (boost::uint32_t)(fDur * 1000.0f);

            ppbox::cdn::Vod2Segment firstseg;
            firstseg.index = 0;
            firstseg.head_length = head_length;
            firstseg.duration = duration;
            firstseg.file_length = file_length;

            boost::uint32_t duration_offset = 0;
            firstseg.duration_offset = duration_offset;
            firstseg.duration_offset_us = (boost::uint64_t)duration_offset * 1000;
            duration_offset += firstseg.duration;

            vod_play_info_.segment.segments.push_back(firstseg);

            return true;
        }

        boost::system::error_code Vod2Segments::segment_url(
            size_t segment, 
            framework::string::Url & url,
            boost::system::error_code & ec)
        {
            ec.clear();
            if (segment < vod_play_info_.drag[0].segments.size()) {
                framework::string::Url cdn_url("http://localhost/");
                cdn_url.host(server_host_.host());
                cdn_url.svc(server_host_.svc());
                cdn_url.path("/" + format(segment) + "/" + name_);
                cdn_url.param("key", get_key());
                cdn_url.param("type", type_);
                cdn_url.param("vvid", vvid_);
                cdn_url.param("platform", platform_);
                LOG_S(framework::logger::Logger::kLevelDebug, "cdn url: " << cdn_url.to_string());
            } else {
                ec = error::item_not_exist;
            }
            return ec;
        }

        void Vod2Segments::response(
            boost::system::error_code const & ec)
        {
            SegmentBase::response_type resp;
            resp.swap(resp_);
            resp(ec);
        }

        std::string Vod2Segments::get_key() const
        {
            return util::protocol::pptv::gen_key_from_time(server_time_ + (Time::now() - local_time_).total_seconds());
        }


        size_t Vod2Segments::segment_count()
        {
            size_t ret = size_t(-1);
            ret = vod_play_info_.segment.segments.size();
            return ret;
        }

        void Vod2Segments::segment_info(
            size_t segment, 
            common::SegmentInfo & info)
        {
            if (segment < vod_play_info_.segment.segments.size()) {
                info.head_size = vod_play_info_.segment.segments[segment].head_length;
                info.size = vod_play_info_.segment.segments[segment].file_length;
                info.time = vod_play_info_.segment.segments[segment].duration;
            } else {
                info.head_size = boost::uint64_t(-1);
                info.size = boost::uint64_t(-1);
                info.time = boost::uint64_t(-1);
            }
        }

        boost::system::error_code Vod2Segments::get_duration(
            common::DurationInfo & info,
            boost::system::error_code & ec)
        {
            ec.clear();
            if (0 != vod_play_info_.video.duration) {
                info.total = vod_play_info_.video.duration;
                info.begin = 0;
                info.end = vod_play_info_.video.duration;
                info.redundancy = 0;
                info.interval = 0;
            } else {
                ec = error::not_open;
            }
            return ec;
        }

        void Vod2Segments::update_segment(size_t segment)
        {
            if (segment_count() == segment ) {
                Vod2Segment vod2_seg;
                vod2_seg.duration = boost::uint32_t(-1);
                vod2_seg.file_length = boost::uint64_t(-1);
                vod2_seg.head_length = boost::uint64_t(-1);
                vod_play_info_.segment.segments.push_back(vod2_seg);
            } else if (segment_count() > segment) {
            } else {
                assert(false);
            }
        }

        void Vod2Segments::update_segment_duration(size_t segment, boost::uint32_t time)
        {
            if (vod_play_info_.segment.segments.size() > segment ) {
                vod_play_info_.segment.segments[segment].duration = time;
            }
        }

        void Vod2Segments::update_segment_file_size(size_t segment, boost::uint64_t fsize)
        {
            if (vod_play_info_.segment.segments.size() > segment) {
                if ( vod_play_info_.segment.segments[segment].file_length == boost::uint64_t(-1)) {
                    vod_play_info_.segment.segments[segment].file_length = fsize;
                }
            }

            segment++;
            if (vod_play_info_.segment.segments.size() == segment) {
                // add a segment
                Vod2Segment vod2_seg;
                vod2_seg.duration = boost::uint32_t(-1);
                vod2_seg.file_length = boost::uint64_t(-1);
                vod2_seg.head_length = boost::uint64_t(-1);
                vod_play_info_.segment.segments.push_back(vod2_seg);
            }
        }

        void Vod2Segments::update_segment_head_size(size_t segment, boost::uint64_t hsize)
        {
            if (vod_play_info_.segment.segments.size() > segment) {
                if (vod_play_info_.segment.segments[segment].head_length == boost::uint64_t(-1)) {
                        vod_play_info_.segment.segments[segment].head_length = hsize;
                }
            }

            segment++;
            if (vod_play_info_.segment.segments.size() == segment) {
                // add a segment
                Vod2Segment vod2_seg;
                vod2_seg.duration = boost::uint32_t(-1);
                vod2_seg.file_length = boost::uint64_t(-1);
                vod2_seg.head_length = boost::uint64_t(-1);
                vod_play_info_.segment.segments.push_back(vod2_seg);
            }
        }

        framework::string::Url Vod2Segments::get_play_url()
        {
            framework::string::Url url("http://localhost/");
            url.host(dns_vod_play_server.host());
            url.svc(dns_vod_play_server.svc());
            url.path("/boxplay.api");
            url.param("auth","55b7c50dc1adfc3bcabe2d9b2015e35c");
            url.param("id",name_);
            if(ft_ != (-1))
            {
                url.param("f", format(ft_));
            }
            url.param("type", type_);
            url.param("vvid", vvid_);
            url.param("platform", platform_);

            LOG_S(framework::logger::Logger::kLevelDebug,"[get_request] play url:"<<url.to_string());

            return url;
        }

        void Vod2Segments::set_url(std::string const &url)
        {
            SegmentBase::set_url(url);

            boost::system::error_code ec;
            std::string::size_type slash = url.find('|');
            if (slash == std::string::npos) {
                return;
            } 
            std::string key = url.substr(0, slash);
            std::string playlink = url.substr(slash + 1);

            framework::string::Url request_url(playlink);
            playlink = request_url.path().substr(1);

            std::string strBwtype = request_url.param("bwtype");
            if(!strBwtype.empty()) {
                bwtype_ = framework::string::parse<boost::int32_t>(strBwtype);
            }
            platform_ = request_url.param("platform");
            if(platform_.empty())
            {
                platform_ = PPBOX_VOD_PLATFORM;
            }

            type_ = request_url.param("type");
            if(type_.empty())
            {
                type_ = PPBOX_VOD_TYPE;
            }
            vvid_ = request_url.param("vvid");
            if(vvid_.empty())
            {
                size_t vvid = rand();
                vvid_ = format(vvid);
            }
            name_ = playlink;
        }


        boost::system::error_code Vod2Segments::reset(size_t & segment)
        {
            segment = 0;
            return boost::system::error_code();
        }

    }//cdn
}//ppbox

//Vod2Segments.cpp