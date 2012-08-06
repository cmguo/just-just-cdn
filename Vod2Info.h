//Vod2Info.h
#ifndef _PPBOX_CDN_VOD2_INFO_H_
#define _PPBOX_CDN_VOD2_INFO_H_

#include "ppbox/cdn/CdnError.h"

#include <ppbox/common/Serialize.h>

#include <util/serialization/Optional.h>
#include <util/serialization/stl/vector.h>
#include <util/archive/XmlIArchive.h>
#include <util/archive/ArchiveBuffer.h>

#include <framework/network/NetName.h>
#include <framework/string/Algorithm.h>

namespace ppbox
{
    namespace cdn
    {

        struct Vod2Segment
        {
            boost::uint32_t index;
            boost::uint32_t duration;   // 分段时长（毫秒）
            boost::uint64_t head_length;
            boost::uint64_t file_length;

            boost::uint32_t duration_offset;    // 相对起始的时长起点，（毫秒）
            boost::uint64_t duration_offset_us; // 同上，（微秒）

            Vod2Segment()
                : index(0)
                , duration(0)
                , head_length(0)
                , file_length(0)
                , duration_offset(0)
                , duration_offset_us(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                float duration = (float)this->duration / 1000.0f;
                ar & util::serialization::make_nvp("hl", head_length)
                    & util::serialization::make_nvp("fs", file_length)
                    & util::serialization::make_nvp("dur", duration)
                    & util::serialization::make_nvp("no", index);
                this->duration = (boost::uint32_t)(duration * 1000.0f);
            }
        };

        struct Vod2SegmentNew
        {
            boost::int32_t ft;
            std::vector<Vod2Segment> segments;
            Vod2SegmentNew()
                :ft(0)
            {
            }
            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar & SERIALIZATION_NVP(ft)
                    & segments;
            }
        };

        struct Vod2DtInfo
        {
            boost::int32_t bwt;
            boost::int32_t ft;
            std::string id;
            framework::network::NetName sh;
            framework::network::NetName bh;
            util::serialization::UtcTime st;

            Vod2DtInfo()
                : bwt(0)
                ,ft(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar & SERIALIZATION_NVP(sh)
                    & SERIALIZATION_NVP(st)
                    & SERIALIZATION_NVP(id)
                    & SERIALIZATION_NVP(bh)
                    & SERIALIZATION_NVP(ft)
                    & SERIALIZATION_NVP(bwt);
            }
        };

        struct Vod2Video
        {
            std::string rid;
            boost::uint32_t bitrate;    // 平均码流率
            boost::int32_t ft;

            boost::uint64_t filesize;
            boost::uint32_t duration;   // 影片时长（微秒）
            boost::uint32_t width;
            boost::uint32_t height;


            Vod2Video()
                : bitrate(0)
                , ft(0)
                , filesize(0)
                , duration(0)
                , width(0)
                , height(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar  & SERIALIZATION_NVP(rid)
                    & SERIALIZATION_NVP(bitrate)
                    & SERIALIZATION_NVP(ft);
            }
        };

        struct Vod2Channel
        {
            //point
            std::vector<Vod2Video> file;

            Vod2Channel()
            {
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar & SERIALIZATION_NVP(file);
            }

        };

        struct VodPlayInfoDrag
        {
            std::vector<Vod2SegmentNew> drag;

            VodPlayInfoDrag()
            {
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar & util::serialization::make_nvp("drag", ar.abnormal_collection(drag));
            }

        };

        struct VodPlayInfo
        {

            Vod2Channel channel;
            std::vector<Vod2DtInfo> dt;
            std::vector<Vod2SegmentNew> drag;
            framework::network::NetName uh;

            Vod2Video video;
            Vod2DtInfo dtinfo;
            Vod2SegmentNew segment;
            boost::int32_t ft;

            bool is_ready;
            boost::system::error_code ec;

            VodPlayInfo()
                :ft(-1)
                ,is_ready(false)
            {
            }

            VodPlayInfo & operator=(
                VodPlayInfoDrag const & r)
            {
                if(ft < 0)
                {
                    return *this;
                }
                for(size_t ii = 0; ii < r.drag.size(); ++ii)
                {
                    if(r.drag[ii].ft == ft)
                    {
                        this->segment = r.drag[ii];
                        break;
                    }
                }
                return *this;
            }

            void finish()
            {
                if(ft == (-1) && !find_ft())
                {
                    ec = error::bad_ft_param;
                    return;
                }

                //VodVideio
                for(size_t ii = 0; ii < channel.file.size(); ++ii)
                {
                    if(channel.file[ii].ft == ft)
                    {
                        video = channel.file[ii];
                        break;
                    }
                }

                //DT info
                for(size_t jj = 0; jj < dt.size(); ++jj)
                {
                    if(dt[jj].ft == ft)
                    {
                        dtinfo = dt[jj];
                        break;
                    }
                }

                //drag

                for(size_t kk = 0; kk < drag.size(); ++kk)
                {
                    if(drag[kk].ft == ft)
                    {
                        segment = drag[kk];

                        boost::uint32_t duration_offset = 0;
                        for (size_t i = 0; i < segment.segments.size(); ++i) 
                        {
                            segment.segments[i].duration_offset = duration_offset;
                            segment.segments[i].duration_offset_us = (boost::uint64_t)duration_offset * 1000;
                            duration_offset += segment.segments[i].duration;
                            video.duration += segment.segments[i].duration;
                        }
                        break;
                    }
                }
                if (segment.segments.size() < 1)
                {
                    ec = error::bad_ft_param;;
                }
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar & SERIALIZATION_NVP(channel)
                    & util::serialization::make_nvp("dt", ar.abnormal_collection(dt))
                    & util::serialization::make_nvp("drag", ar.abnormal_collection(drag))
                    & SERIALIZATION_NVP(uh);

                finish();
            }
        private:
            bool find_ft()
            {
                if(channel.file.size() < 1) return false;
                //channel.file.bitrate
                boost::uint32_t iBitrate = channel.file[0].bitrate;
                boost::int32_t iFt = channel.file[0].ft;
                for(size_t ii = 1; ii < channel.file.size(); ++ii)
                {
                    if(channel.file[ii].bitrate > iBitrate)
                    {
                        iBitrate = channel.file[ii].bitrate;
                        iFt = channel.file[ii].ft;
                    }
                }
                ft = iFt;
                return true;
            }
        };
    }//cdn
}//ppbox


#endif
//_PPBOX_CDN_VOD2_INFO_H_