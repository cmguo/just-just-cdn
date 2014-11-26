// PptvMediaInfo.h

#ifndef _PPBOX_CDN_PPTV_PPTV_MEDIA_INFO_H_
#define _PPBOX_CDN_PPTV_PPTV_MEDIA_INFO_H_

#include "ppbox/cdn/UtcTime.h"
#include "ppbox/cdn/Serialize.h"

#include <ppbox/data/base/MediaBase.h>
#include <ppbox/data/segment/SegmentInfo.h>

#include <util/serialization/Optional.h>

#include <framework/network/NetName.h>

#ifndef PPBOX_CDN_PARAM_DELIM
#  define PPBOX_CDN_PARAM_DELIM ","
#endif

namespace ppbox
{
    namespace cdn
    {

        using ppbox::data::invalid_size;

        enum VersionEnum
        {
            vod_1 = 1, 
            vod_quick, 
            vod_play, 
            live_1, 
            live_2, 
            live_2_play, 
        };

        struct Video
            : ppbox::data::MediaInfo
        {
            std::string rid;

            Video()
            {
                flags |= f_segment;
                flags |= ppbox::data::SegmentMediaFlags::f_segment_seek;
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                switch (ar.version()) {
                    case vod_1:
                    case vod_quick:
                        {
                            float duration = (float)this->duration / 1000.0f;
                            boost::uint32_t bitrate = this->bitrate / 8;
                            ar & SERIALIZATION_NVP(name)
                                & SERIALIZATION_NVP_NAME("filesize", file_size)
                                & SERIALIZATION_NVP(bitrate)
                                & SERIALIZATION_NVP(duration);
                            this->duration = (boost::uint32_t)(duration * 1000.0f);
                            this->bitrate = bitrate * 8;
                        }
                        break;
                    case live_2:
                        ar & SERIALIZATION_NVP_NAME("channelGUID", rid);
                        break;
                    case vod_play: // 点播Play
                    case live_2_play: // 二代直播Play
                        {
                            boost::uint32_t bitrate = this->bitrate / 1000;
                            ar & SERIALIZATION_NVP(rid)
                                & SERIALIZATION_NVP(bitrate);
                            this->bitrate = bitrate * 1000;
                        }
                        break;
                    default:
                        ar.fail();
                        break;
                }
            }

            friend bool operator==(
                Video const & l, 
                Video const & r)
            {
                return l.rid == r.rid
                    && l.bitrate == r.bitrate
                    && l.duration == r.duration
                    && l.file_size == r.file_size;
            }
        };

        struct Jump
        {
            framework::network::NetName server_host;
            UtcTime server_time;
            size_t bw_type;
            framework::network::NetName back_host;

            Jump()
                : server_host("", 80)
                , bw_type(0)
                , back_host("", 80)
            {
            }

            template <
                typename Archive
            >
            void serialize( 
                Archive & ar)
            {
                switch (ar.version()) {
                    case vod_1:
                    case vod_quick:
                        ar & SERIALIZATION_NVP(server_host)
                            & SERIALIZATION_NVP(server_time)
                            & SERIALIZATION_NVP_NAME("BWType", bw_type);
                        break;
                    case live_2:
                        ar & SERIALIZATION_NVP(server_host)
                            & SERIALIZATION_NVP(server_time);
                        break;
                    case vod_play: // 点播Play
                    case live_2_play: // 二代直播Play
                        ar & SERIALIZATION_NVP_NAME("sh", server_host)
                            & SERIALIZATION_NVP_NAME("st", server_time)
                            & SERIALIZATION_NVP_NAME("bwt", bw_type)
                            & SERIALIZATION_NVP_NAME("bh", util::serialization::make_optional(back_host));
                        break;
                    default:
                        ar.fail();
                        break;
                }
            }

            friend bool operator==(
                Jump const & l, 
                Jump const & r)
            {
                return l.server_host == r.server_host
                    && l.bw_type == r.bw_type
                    && l.back_host == r.back_host;
            }
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_PPTV_PPTV_MEDIA_INFO_H_
