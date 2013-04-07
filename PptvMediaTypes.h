// PptvMeidaTypes.h

#ifndef _PPBOX_CDN_PPTV_MEDIA_TYPES_H_
#define _PPBOX_CDN_PPTV_MEDIA_TYPES_H_

#include <ppbox/common/ClassRegister.h>

#include "ppbox/cdn/PptvVod1.h"
#include "ppbox/cdn/PptvVod2.h"
#include "ppbox/cdn/PptvLive1.h"
#include "ppbox/cdn/PptvLive2.h"
#include "ppbox/cdn/PptvLive3.h"

namespace ppbox
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

        PPBOX_REGISTER_CLASS_FACTORY_FUNC("pptv", ppbox::data::MediaProtocolFactory, create_pptv);

    } // cdn
} // ppbox

#endif // _PPBOX_CDN_PPTV_MEDIA_TYPES_H_
