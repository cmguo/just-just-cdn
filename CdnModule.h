// CdnModule.h

#ifndef _JUST_CDN_CDN_MODULE_H_
#define _JUST_CDN_CDN_MODULE_H_

#include <just/common/CommonModuleBase.h>

namespace just
{
    namespace cdn
    {

        class CdnModule
            : public just::common::CommonModuleBase<CdnModule>
        {
        public:
            CdnModule(
                util::daemon::Daemon & daemon);

            ~CdnModule();

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();
        };

    } // namespace cdn
} // namespace just

#endif // _JUST_CDN_CDN_MODULE_H_
