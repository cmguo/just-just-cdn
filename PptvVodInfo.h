// PptvVodInfo.h

#ifndef _PPBOX_CDN_PPTV_VOD_INFO_H_
#define _PPBOX_CDN_PPTV_VOD_INFO_H_

#include "ppbox/cdn/PptvMediaInfo.h"

namespace ppbox
{
    namespace cdn
    {

        struct VodSegment
        {
            boost::uint64_t head_length;
            boost::uint64_t file_length;
            boost::uint32_t duration;   // 分段时长（毫秒）
            boost::uint64_t offset;

            VodSegment()
                : head_length(0)
                , file_length(0)
                , duration(0)
                , offset(0)
            {
            }

            template <
                typename Archive
            >
            void serialize(
            Archive & ar)
            {
                if (ar.version() == 0) { // 猜测Drag版本
                    boost::optional<boost::uint32_t> head_length;
                    char const * const names[] = {"headlength", "h", "hl"};
                    for (size_t i = 0; i < 3; ++i) {
                        ar & util::serialization::make_nvp(names[i], head_length);
                        ar.version(i + 1);
                        break;
                    }
                }

                if (ar.version() == 0) {
                    ar.fail();
                    return;
                }

                float duration = (float)this->duration / 1000.0f;

                switch (ar.version()) {
                    case vod_1:
                        ar & util::serialization::make_nvp("headlength", head_length)
                            & util::serialization::make_nvp("filesize", file_length)
                            & util::serialization::make_nvp("duration", duration)
                            & util::serialization::make_nvp("offset", offset);
                        break;
                    case vod_quick:
                        ar & util::serialization::make_nvp("h", head_length)
                            & util::serialization::make_nvp("f", file_length)
                            & util::serialization::make_nvp("d", duration);
                        break;
                    case vod_play:
                        ar & util::serialization::make_nvp("hl", head_length)
                            & util::serialization::make_nvp("fs", file_length)
                            & util::serialization::make_nvp("dur", duration);
                        break;
                }

                this->duration = (boost::uint32_t)(duration * 1000.0f);
            }
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_PPTV_VOD_INFO_H_
