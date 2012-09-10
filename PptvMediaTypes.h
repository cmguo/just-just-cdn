// PptvMeidaTypes.h

#ifndef _PPBOX_CDN_PPTV_MEDIA_TYPES_H_
#define _PPBOX_CDN_PPTV_MEDIA_TYPES_H_

#include "ppbox/cdn/PptvVod.h"
#include "ppbox/cdn/PptvVod2.h"
#include "ppbox/cdn/PptvLive2.h"

namespace ppbox
{
    namespace cdn
    {

        PptvMedia * pptv_create_media(
            boost::asio::io_service & io_svc)
        {
            assert(0);
            return NULL;
        }

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_PPTV_MEDIA_TYPES_H_
