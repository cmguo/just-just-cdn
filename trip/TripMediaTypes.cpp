// PptvMeidaTypes.h

#ifndef _PPBOX_CDN_PPTV_PPTV_MEDIA_TYPES_H_
#define _PPBOX_CDN_PPTV_PPTV_MEDIA_TYPES_H_

#include "ppbox/cdn/Common.h"
#include "ppbox/cdn/pptv/PptvLive1.h"

#include <util/daemon/Daemon.h>
#include <util/protocol/pptv/Base64.h>
using namespace util::protocol;

#include <framework/string/Parse.h>
using namespace framework::string;

namespace ppbox
{
    namespace cdn
    {

        ppbox::data::MediaBase * create_trip(
            boost::asio::io_service & io_svc,
            Url const & url)
        {
            std::string key = url.param_or("cdn.key", "trinity");
            std::string url_str = url.path().substr(1);
            url_str = pptv::base64_decode(url_str, key);
            if (url_str.empty()) {
                return NULL;
            }
            std::string appid, expire;
            map_find(url_str, "ap", appid, "&");
            map_find(url_str, "et", expire, "&");
            if (!appid.empty()) {
                util::daemon::Daemon & daemon = 
                    util::daemon::Daemon::from_io_svc(io_svc);
                std::string appid1;
                daemon.config().get("TripLiveMedia", "appid", appid1);
                if (appid != appid1) {
                    return NULL;
                }
            };
            if (!expire.empty()) {
                if (time(NULL) > parse<time_t>(expire)) {
                    return NULL;
                }
            }
            Url url1(url);
            url1.protocol("pplive");
            url1.param("cdn.key", key);
            return new PptvLive1(io_svc, url1);
        }

        UTIL_REGISTER_CLASS_FUNC(ppbox::data::MediaProtocolFactory, "trip", create_trip);

    } // cdn
} // ppbox

#endif // _PPBOX_CDN_PPTV_PPTV_MEDIA_TYPES_H_
