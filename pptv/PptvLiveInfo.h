// PptvLiveInfo.h

#ifndef _JUST_CDN_PPTV_PPTV_LIVE_INFO_H_
#define _JUST_CDN_PPTV_PPTV_LIVE_INFO_H_

#include "just/cdn/pptv/PptvMediaInfo.h"

namespace just
{
    namespace cdn
    {

        struct LiveVideo
            : Video
        {
            LiveVideo()
            {
                flags |= just::data::SegmentMediaFlags::f_time_smoth;
                flags |= just::data::SegmentMediaFlags::f_fix_duration;
            }
        };

        struct LiveSegment
        {
            boost::uint16_t interval;

            LiveSegment()
                : interval(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                assert(ar.version() == live_2_play);
                ar  & SERIALIZATION_NVP(interval);
            }
        };

    } // namespace cdn
} // namespace just

#endif//_JUST_CDN_PPTV_PPTV_LIVE_INFO_H_
