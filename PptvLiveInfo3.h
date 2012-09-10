// PptvLiveInfo3.h

#ifndef _PPBOX_CDN_PPTV_LIVE_INFO3_H_
#define _PPBOX_CDN_PPTV_LIVE_INFO3_H_

#include "ppbox/cdn/PptvLiveInfo.h"

#include <util/serialization/stl/vector.h>

namespace ppbox
{
    namespace cdn
    {

        struct Live2Channel
        {
            //point
            std::string nm;
            size_t jump;
            size_t delay;
            LiveSegment seg;
            std::vector<Video> stream;

            Live2Channel()
                : jump(0)
                , delay(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                ar & SERIALIZATION_NVP(nm);
                ar & SERIALIZATION_NVP(jump);
                ar & SERIALIZATION_NVP(delay);
                ar & seg;
                ar & SERIALIZATION_NVP(stream);
            }

        };

        struct LivePlayInfo
        {
            Live2Channel channel;
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
