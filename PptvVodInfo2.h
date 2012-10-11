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
            size_t ft;
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
                ar & SERIALIZATION_NVP(ft)
                    & segments;
            }
        };

        struct Vod2Jump
            : Jump
        {
            size_t ft;

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
                ar & SERIALIZATION_NVP(ft);
                Jump::serialize(ar);
            }
        };

        struct Vod2Video
            : public Video
        {
            size_t ft;

            Vod2Video()
                : ft(0)
            {
            }

            bool operator<(
                Vod2Video const & r)
            {
                return ft < r.ft;
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                ar  & SERIALIZATION_NVP(ft);
                Video::serialize(ar);
            }
        };

        struct Vod2Channel
        {
            //point
            std::string nm;
            boost::uint32_t dur;    // 影片时长（微秒）
            boost::uint32_t ts;     // 影片字节长度
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
                ar & SERIALIZATION_NVP(nm);
                ar & SERIALIZATION_NVP(dur);
                ar & SERIALIZATION_NVP(ts);
                ar & SERIALIZATION_NVP(file);
            }

        };

        struct VodPlayInfo
        {
            Vod2Channel channel;
            std::vector<Vod2Jump> jumps;
            std::vector<Vod2Drag> drags;
            std::string uh;

            VodPlayInfo()
            {
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                ar.version(vod_play);
                ar & SERIALIZATION_NVP(channel)
                    & util::serialization::make_nvp("dt", ar.abnormal_collection(jumps))
                    & util::serialization::make_nvp("drag", ar.abnormal_collection(drags))
                    & SERIALIZATION_NVP(uh);
            }
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_VOD2_INFO_H_
