// PptvLiveInfo3.h

#ifndef _PPBOX_CDN_PPTV_LIVE_INFO3_H_
#define _PPBOX_CDN_PPTV_LIVE_INFO3_H_

#include "ppbox/cdn/PptvLiveInfo.h"

#include <util/serialization/stl/vector.h>

namespace ppbox
{
    namespace cdn
    {

        struct Live3Video
            : public Video
        {
            boost::int32_t ft;

            Live3Video()
                : ft(0)
            {
            }

            bool operator<(
                Live3Video const & r)
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
            size_t delay;
            size_t jump;
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
                    & util::serialization::make_nvp("dt", jump)
                    & SERIALIZATION_NVP(uh);
            }
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_PPTV_LIVE_INFO3_H_
