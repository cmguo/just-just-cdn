// PptvVodInfo1.h

#ifndef _PPBOX_CDN_PPTV_VOD_INFO1_H_
#define _PPBOX_CDN_PPTV_VOD_INFO1_H_

#include "ppbox/cdn/PptvVodInfo.h"

#include <util/serialization/stl/vector.h>

namespace ppbox
{
    namespace cdn
    {

        struct VodJumpInfo
            : public Jump
        {
            VodJumpInfo()
            {
            }

            std::string user_host;
            boost::optional<Video> video;
            boost::optional<VodSegment> firstseg;

            template <
                typename Archive
            >
            void serialize( 
                Archive & ar)
            {
                ar.version(vod);
                Jump::serialize(ar);
                ar & SERIALIZATION_NVP(user_host);

                ar & SERIALIZATION_NVP(video);
                if (video.is_initialized()) {
                    ar.version(vod_quick);
                    ar & SERIALIZATION_NVP(firstseg);
                }
            }
        };

        struct VodDragInfo
        {
            Video video;
            std::vector<VodSegment> segments;

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                ar.version(vod);
                ar & SERIALIZATION_NVP(video);
                ar & util::serialization::make_nvp("segments", util::serialization::make_optional(segments, vod));
                if (segments.empty())
                    ar & util::serialization::make_nvp("ss", util::serialization::make_optional(segments, vod_quick));
            }

        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_PPTV_VOD_INFO1_H_
