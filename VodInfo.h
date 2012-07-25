// VodInfo.h

#ifndef _PPBOX_CDN_VOD_INFO_H_
#define _PPBOX_CDN_VOD_INFO_H_

#include "ppbox/common/Serialize.h"

#include <util/serialization/stl/vector.h>
#include <util/serialization/Optional.h>
#include <util/archive/XmlIArchive.h>
#include <util/archive/ArchiveBuffer.h>

#include <framework/network/NetName.h>

namespace ppbox
{
    namespace cdn
    {
        struct VodSegment
        {
            boost::uint64_t head_length;
            boost::uint64_t file_length;
            boost::uint32_t duration;   // 分段时长（毫秒）
            std::string va_rid;
            boost::uint32_t duration_offset;    // 相对起始的时长起点，（毫秒）
            boost::uint64_t duration_offset_us; // 同上，（微秒）
            boost::uint64_t block_size;
            boost::uint32_t block_num;

            VodSegment()
                : head_length(0)
                , file_length(0)
                , duration(0)
                , duration_offset(0)
                , duration_offset_us(0)
                , block_size(0)
                , block_num(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                float duration = (float)this->duration / 1000.0f;
                ar & util::serialization::make_nvp("headlength", head_length)
                    & util::serialization::make_nvp("filesize", file_length)
                    & util::serialization::make_nvp("duration", duration)
                    & util::serialization::make_nvp("varid", va_rid);
                this->duration = (boost::uint32_t)(duration * 1000.0f);
            }
        };

        struct VodSegmentNew
            : VodSegment
        {
            VodSegmentNew()
                : VodSegment()
            {
            }

            VodSegmentNew(
                VodSegment const & r)
                : VodSegment(r)
            {
                std::string rid;
                map_find(r.va_rid, "rid", rid, "&");
                map_find(r.va_rid, "blocksize", this->block_size, "&");
                map_find(r.va_rid, "blocknum", this->block_num, "&");

                this->va_rid = rid;
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                float duration = (float)this->duration / 1000.0f;
                ar & util::serialization::make_nvp("h", head_length)
                    & util::serialization::make_nvp("f", file_length)
                    & util::serialization::make_nvp("d", duration)
                    & util::serialization::make_nvp("r", va_rid);
                this->duration = (boost::uint32_t)(duration * 1000.0f);
            }
        };

        struct VodVideo
        {
            std::string name;
            std::string type;
            boost::uint32_t bitrate;    // 平均码流率
            boost::uint64_t filesize;
            boost::uint32_t duration;   // 影片时长（微秒）
            boost::uint32_t width;
            boost::uint32_t height;

            VodVideo()
                : bitrate(0)
                , filesize(0)
                , duration(0)
                , width(0)
                , height(0)
            {
            }
            // 
            //             void operator = (VodVideo & video)
            //             {
            //                 this->name = video.name;
            //                 this->type = video.type;
            //                 this->bitrate = video.bitrate;
            //                 this->filesize = video.filesize;
            //                 this->duration = video.duration;
            //                 this->width = video.width;
            //                 this->height = video.height;
            //            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                float duration = (float)this->duration / 1000.0f;
                ar & SERIALIZATION_NVP(name)
                    & SERIALIZATION_NVP(type)
                    & SERIALIZATION_NVP(bitrate)
                    & SERIALIZATION_NVP(filesize)
                    & SERIALIZATION_NVP(duration)
                    & SERIALIZATION_NVP(width)
                    & SERIALIZATION_NVP(height);
                this->duration = (boost::uint32_t)(duration * 1000.0f);
            }
        };

        struct VodJumpInfo
        {
            VodJumpInfo()
                : server_host("", 80)
                , BWType(0)
                , block_size(0)
            {
            }

            framework::network::NetName user_host;
            framework::network::NetName server_host;
            util::serialization::UtcTime server_time;
            std::string id;
            int BWType;

            boost::optional<VodVideo> video;
            boost::optional<boost::uint64_t> block_size;
            boost::optional<VodSegmentNew> firstseg;

            template <
                typename Archive
            >
            void serialize( 
            Archive & ar)
            {
                ar & SERIALIZATION_NVP(user_host)
                    & SERIALIZATION_NVP(server_host)
                    & SERIALIZATION_NVP(server_time)
                    & SERIALIZATION_NVP(id)
                    & SERIALIZATION_NVP(BWType)
                    & SERIALIZATION_NVP(video)
                    & util::serialization::make_nvp("block_size", block_size)
                    & SERIALIZATION_NVP(firstseg);

                if (firstseg.is_initialized()) {
                    boost::uint32_t duration_offset = 0;
                    firstseg->duration_offset = duration_offset;
                    firstseg->duration_offset_us = (boost::uint64_t)duration_offset * 1000;
                    duration_offset += firstseg->duration;
                    //                    firstseg->block_size = block_size;
                    //                    if (block_size != 0)
                    //                        firstseg->block_num = (boost::uint32_t)(firstseg->file_length / block_size + ((firstseg->file_length % block_size != 0) ? 1 : 0));
                }

            }
        };

        typedef std::vector<VodSegment> VodSegmentsOld;

        struct VodSegmentsNew
            : public std::vector<VodSegmentNew>
        {
            boost::uint64_t blocksize;

            VodSegmentsNew()
                : blocksize(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar & SERIALIZATION_NVP(blocksize)
                    & (std::vector<VodSegmentNew> &)(*this);
                if (!ar)
                    return;
                for (size_t i = 0; i < size(); ++i) {
                    VodSegmentNew & seg = at(i);
                    seg.block_size = blocksize;
                    if (seg.block_size != 0) {
                        seg.block_num = (boost::uint32_t)(
                            seg.file_length / seg.block_size + 
                            ((seg.file_length % seg.block_size != 0) ? 1 : 0));
                    }
                }
            }
        };

        struct VodDragInfo
        {
            VodVideo video;
            std::vector<VodSegment> segments;
            boost::optional<VodSegmentsOld> segments_old;
            boost::optional<VodSegmentsNew> segments_new;

            template <typename Archive>
            void serialize(Archive & ar)
            {
                ar & SERIALIZATION_NVP(video)
                    & util::serialization::make_nvp("segments", segments_old)
                    & util::serialization::make_nvp("ss", segments_new);

                if (segments_old.is_initialized()) {
                    segments.insert(segments.end(), segments_old->begin(), segments_old->end());
                } else if(segments_new.is_initialized()) {
                    segments.insert(segments.end(), segments_new->begin(), segments_new->end());
                } else {
                    ar.fail();
                    return;
                }
                boost::uint32_t duration_offset = 0;
                for (size_t i = 0; i < segments.size(); ++i) {
                    segments[i].duration_offset = duration_offset;
                    segments[i].duration_offset_us = (boost::uint64_t)duration_offset * 1000;
                    duration_offset += segments[i].duration;
                }
            }

        };

        /************************************************************************/
        /*                            play结构定义                            */
        /************************************************************************/ 


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

            void operator = (std::vector<Vod2Segment> & seg)
            {
                std::vector<Vod2Segment>::iterator iter = seg.begin();
                while (seg.end() != iter) {
                    this->segments.push_back(*iter);
                    iter ++;
                }
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
            }
        };


    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_VOD_VOD_INFO_H_
