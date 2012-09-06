// PptvMeidaInfo.h

#ifndef _PPBOX_CDN_CDN_INFO_H_
#define _PPBOX_CDN_CDN_INFO_H_

#include "ppbox/cdn/UtcTime.h"

#include <ppbox/data/MediaBase.h>

#include <util/serialization/Optional.h>
#include <util/archive/XmlIArchive.h>
#include <util/archive/ArchiveBuffer.h>

#include <framework/network/NetName.h>

namespace util
{
    namespace serialization
    {
        template <
            typename Archive
        >
        void serialize(
            Archive & ar, 
            framework::network::NetName & t)
        {
            std::string value;
            ar & value;
            t.from_string(value);
        }

    }
}

namespace ppbox
{
    namespace cdn
    {

        enum VersionEnum
        {
            vod, 
            vod_quick, 
            vod_play, 
            live, 
            live_2, 
            live_2_play, 
        };

        struct Video
            : ppbox::data::MediaInfo
        {
            std::string rid;

            Video()
            {
            }

            template <
                typename Archive
            >
            void serialize(
                Archive & ar)
            {
                switch (ar.version()) {
                    case vod:
                    case vod_quick:
                        {
                            float duration = (float)this->duration / 1000.0f;
                            ar & SERIALIZATION_NVP(name)
                                & SERIALIZATION_NVP(bitrate)
                                & SERIALIZATION_NVP(duration);
                            this->duration = (boost::uint32_t)(duration * 1000.0f);
                        }
                        break;
                    case live_2:
                        ar & SERIALIZATION_NVP_NAME("channelGUID", rid);
                        break;
                    case vod_play: // 点播Play
                    case live_2_play: // 二代直播Play
                        ar & SERIALIZATION_NVP(rid)
                            & SERIALIZATION_NVP(bitrate);
                        break;
                    default:
                        ar.fail();
                        break;
                }
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
                    case vod:
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
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_CDN_INFO_H_
