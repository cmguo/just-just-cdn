// PptvLiveInfo3.h

#ifndef _PPBOX_CDN_PPTV_PPTV_LIVE_INFO3_H_
#define _PPBOX_CDN_PPTV_PPTV_LIVE_INFO3_H_

#include "ppbox/cdn/pptv/PptvLiveInfo.h"

#include <util/serialization/stl/vector.h>

namespace ppbox
{
    namespace cdn
    {

        struct Live3Video
            : public LiveVideo
        {
            size_t ft;

            Live3Video()
                : ft(0)
            {
            }

            bool operator<(
                Live3Video const & r) const
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

        struct Live3Stream
        {
            boost::uint32_t delay;
            boost::uint32_t jump;
            LiveSegment seg; // interval
            std::vector<Live3Video> item;

            Live3Stream()
                : delay(0)
                , jump(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                ar & SERIALIZATION_NVP(delay);
                ar & SERIALIZATION_NVP(jump);
                ar & seg;
                ar & item;
                delay *= 1000;
                jump *= 1000;
            }

        };

        struct Live3Channel
        {
            std::string nm;
            Live3Stream stream;

            Live3Channel()
            {
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar & SERIALIZATION_NVP(nm);
                ar & SERIALIZATION_NVP(stream);
            }

        };

        struct LivePlayInfo
        {
            Live3Channel channel;
            Jump jump;
            std::string uh;

            LivePlayInfo()
            {
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                ar.version(live_2_play);
                ar & SERIALIZATION_NVP(channel)
                    & SERIALIZATION_NVP_NAME("dt", jump)
                    & SERIALIZATION_NVP(uh);
            }
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_PPTV_PPTV_LIVE_INFO3_H_
