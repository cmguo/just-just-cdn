// CdnModule.h

#ifndef _PPBOX_CDN_CDN_MODULE_H_
#define _PPBOX_CDN_CDN_MODULE_H_

#include <ppbox/common/CommonModuleBase.h>

namespace ppbox
{
    namespace cdn
    {

        class CdnModule
            : public ppbox::common::CommonModuleBase<CdnModule>
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
} // namespace ppbox

#endif // _PPBOX_CDN_CDN_MODULE_H_
