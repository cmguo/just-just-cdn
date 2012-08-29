//Vod2Segments.cpp

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/Vod2Segments.h"
#include "ppbox/cdn/CdnError.h"

#include <ppbox/cdn/HttpFetch.h>

#include <util/protocol/pptv/Url.h>
#include <util/protocol/pptv/TimeKey.h>
#include <util/archive/XmlIArchive.h>
#include <util/archive/ArchiveBuffer.h>
#include <util/buffers/BufferCopy.h>

#include <framework/string/Slice.h>
#include <framework/string/Url.h>
#include <framework/string/Parse.h>
using namespace framework::string;
#include <framework/logger/StreamRecord.h>
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
            : PptvSegments(io_svc)
            , open_step_(StepType::not_open)
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

            set_response(resp);

            open_logs_.resize(1);

            if (parse()) {
                if(vod_play_info_.video.duration == vod_play_info_.segment.segments[0].duration) {
                    vod_play_info_.is_ready = true;
                    open_step_ = StepType::finish;
                    response(ec);
                    return;
                } else {
                    open_step_ = StepType::play;
                }
                if (OpenMode::fast == mode) {
                    response(ec);
                    return;
                }
            }
            handle_async_open(ec);
        }

        void Vod2Segments::handle_async_open(
            boost::system::error_code const & ec)
        {
            if (ec) {
                LOG_WARN("play: failure");
                last_error_ = ec;
                open_logs_end(0, ec);
                response(ec);
                return;
            }

            switch (open_step_) {
            case StepType::play:
                {
                    LOG_INFO("play: start");
                    async_fetch(
                        get_play_url(),
                        dns_vod_play_server,
                        vod_play_info_, 
                        boost::bind(&Vod2Segments::handle_async_open, this, _1));
                    return;
                }
            case StepType::finish:
                {
                   break;
                }
            default:
                return;
            }

            last_error_ = ec;

            response(ec);
        }

        bool Vod2Segments::parse()
        {//验证串第一段信息是否完整，完整先播第一段，否则就先请求play
            //name
            std::string tmp_param = url_.param("f");
            if(tmp_param.empty())
            {
                tmp_param = jdp_url_.param("ft");
                if(tmp_param.empty())
                {
                    LOG_DEBUG("parse ft or f failed");
                    return false;
                }
            }
            parse2(tmp_param.c_str(), vod_play_info_.ft);

            //总时长
            tmp_param = url_.param("duration");
            if(tmp_param.empty())
            {
                LOG_DEBUG("parse duration failed");
                return false;
            }
            float fTotalDur = 0.0;
            parse2(tmp_param.c_str(), fTotalDur);
            vod_play_info_.video.duration = (boost::uint32_t)(fTotalDur*1000.0f);

            //rid
            tmp_param = url_.param("name");
            if(tmp_param.empty())
            {
                LOG_DEBUG("parse name failed");
                return false;
            }
            vod_play_info_.video.rid = tmp_param;

            //server host
            tmp_param = url_.param("svrhost");
            if(tmp_param.empty())
            {
                LOG_DEBUG("parse svrhost failed");
                return false;
            }
            framework::network::NetName svrhost(tmp_param.c_str());
            vod_play_info_.dtinfo.sh = svrhost;

            //server time
            tmp_param = url_.param("svrtime");
            if(tmp_param.empty())
            {
                LOG_DEBUG("parse svrtime failed");
                return false;
            }
            vod_play_info_.dtinfo.st.from_string(tmp_param);

            //bwtype
            tmp_param = url_.param("BWType");
            if(tmp_param.empty())
            {
                LOG_DEBUG("parse bwtype failed");
                return false;
            }
            parse2(tmp_param.c_str(), vod_play_info_.dtinfo.bwt);

            //drag
            tmp_param = url_.param("drag");
            if(tmp_param.empty())
            {
                LOG_DEBUG("parse drag failed");
                return false;
            }
            std::vector<std::string> strs;
            slice<std::string>(tmp_param, std::inserter(strs, strs.end()), "|");
            if(strs.size() != 3)
            {
                LOG_DEBUG("parse drag info failed");
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
                url = cdn_url_;
                url.host(vod_play_info_.dtinfo.sh.host());
                url.svc(vod_play_info_.dtinfo.sh.svc());
                url.path("/" + format(segment) + cdn_url_.path());
                url.param("key", get_key());
                LOG_DEBUG( "cdn url: " << url.to_string());

                framework::string::Url cdn_jump_param(url_.param("cdn.jump"));
                if (cdn_jump_param.param("bwtype").empty()) {
                    cdn_jump_param.param("bwtype", format(vod_play_info_.dtinfo.bwt));
                    url.param("cdn.jump", cdn_jump_param.to_string());
                } 
            } else {
                ec = error::item_not_exist;
            }
            return ec;
        }


        std::string Vod2Segments::get_key() const
        {
            return util::protocol::pptv::gen_key_from_time(
                vod_play_info_.dtinfo.st.to_time_t() + (Time::now() - local_time_).total_seconds());
        }


        size_t Vod2Segments::segment_count() const
        {
            size_t ret = size_t(-1);
            ret = vod_play_info_.segment.segments.size();
            return ret;
        }

        void Vod2Segments::segment_info(
            size_t segment, 
            ppbox::data::SegmentInfo & info) const
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
            ppbox::data::DurationInfo & info,
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

        framework::string::Url Vod2Segments::get_play_url()
        {
            framework::string::Url url = jdp_url_;
            url.host(dns_vod_play_server.host());
            url.svc(dns_vod_play_server.svc());
            url.param("id",url.path().substr(1));
            url.path("/boxplay.api");
            url.param("auth","55b7c50dc1adfc3bcabe2d9b2015e35c");
            if(vod_play_info_.video.ft != (-1))
            {
                url.param("f", format(vod_play_info_.video.ft));
            }

            LOG_DEBUG("[get_request] play url:"<<url.to_string());

            return url;
        }

        void Vod2Segments::set_url(
            framework::string::Url const & url)
        {
            set_url(url);
        }


    }//cdn
}//ppbox

//Vod2Segments.cpp