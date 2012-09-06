// PptvVodInfo2.h

#ifndef _PPBOX_CDN_VOD2_INFO_H_
#define _PPBOX_CDN_VOD2_INFO_H_

#include "ppbox/cdn/PptvVodInfo.h"

#include <util/serialization/stl/vector.h>

namespace ppbox
{
    namespace cdn
    {

        struct Vod2Drag
        {
            boost::int32_t ft;
            std::vector<VodSegment> segments;
            Vod2Drag()
                :ft(0)
            {
            }
            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar.version(3);
                ar & SERIALIZATION_NVP(ft)
                    & segments;
            }
        };

        struct Vod2Jump
            : Jump
        {
            boost::int32_t ft;

            Vod2Jump()
                : ft(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                ar.version(3);
                ar & SERIALIZATION_NVP(ft);
                Jump::serialize(ar);
            }
        };

        struct Vod2Video
            : public Video
        {
            std::string rid;
            boost::uint32_t bitrate;    // 平均码流率
            boost::int32_t ft;

            Vod2Video()
                : bitrate(0)
                , ft(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                ar.version(3);
                ar  & SERIALIZATION_NVP(ft);
                Video::serialize(ar);
            }
        };

        struct Vod2Channel
        {
            //point
            boost::uint32_t duration;   // 影片时长（微秒）
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
                ar & SERIALIZATION_NVP(duration);
                ar & SERIALIZATION_NVP(file);
            }

        };

        struct Vod2PlayInfo
        {
            Vod2Channel channel;
            std::vector<Vod2Jump> jumps;
            std::vector<Vod2Drag> drags;
            framework::network::NetName uh;

            Vod2PlayInfo()
            {
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                ar & SERIALIZATION_NVP(channel)
                    & util::serialization::make_nvp("dt", ar.abnormal_collection(jumps))
                    & util::serialization::make_nvp("drag", ar.abnormal_collection(drags))
                    & SERIALIZATION_NVP(uh);
            }
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_VOD2_INFO_H_
