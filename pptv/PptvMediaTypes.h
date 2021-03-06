// PptvMeidaTypes.h

#ifndef _JUST_CDN_PPTV_PPTV_MEDIA_TYPES_H_
#define _JUST_CDN_PPTV_PPTV_MEDIA_TYPES_H_

#include <util/tools/ClassRegister.h>

#include "just/cdn/pptv/PptvVod1.h"
#include "just/cdn/pptv/PptvVod2.h"
#include "just/cdn/pptv/PptvLive1.h"
#include "just/cdn/pptv/PptvLive2.h"
#include "just/cdn/pptv/PptvLive3.h"

namespace just
{
    namespace cdn
    {

        static PptvMedia * create_pptv(
            boost::asio::io_service & io_svc,
            framework::string::Url const & url)
        {
            // 1 - vod, 2 - live, 5 - live2
            std::string ft = url.param_or("vt", "1");
            if (ft == "1") {
                return new PptvVod2(io_svc, url);
            } else if (ft == "5") {
                return new PptvLive3(io_svc, url);
            } else {
                return NULL;
            }
        }

        UTIL_REGISTER_CLASS_FUNC(just::data::MediaProtocolFactory, "pptv", create_pptv);

    } // cdn
} // just

#endif // _JUST_CDN_PPTV_PPTV_MEDIA_TYPES_H_
