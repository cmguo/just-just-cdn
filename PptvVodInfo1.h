// PptvVodInfo1.h

#ifndef _PPBOX_CDN_VOD1_INFO_H_
#define _PPBOX_CDN_VOD1_INFO_H_

#include "ppbox/cdn/PptvVodInfo.h"

#include <util/serialization/stl/vector.h>

namespace ppbox
{
    namespace cdn
    {

        struct VodJumpInfo
        {
            VodJumpInfo()
            {
            }

            Jump jump;
            framework::network::NetName user_host;
            boost::optional<Video> video;
            boost::optional<VodSegment> firstseg;

            template <
                typename Archive
            >
            void serialize( 
            Archive & ar)
            {
                ar.version(1);
                ar & jump
                    & SERIALIZATION_NVP(user_host);

                ar & SERIALIZATION_NVP(video);
                if (video.is_initialized()) {
                    ar.version(2);
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
                ar & SERIALIZATION_NVP(video);
                ar & util::serialization::make_nvp("segments", util::serialization::make_optional(segments, 1));
                    & util::serialization::make_nvp("ss", util::serialization::make_optional(segments, 2));
            }

        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_VOD1_INFO_H_
