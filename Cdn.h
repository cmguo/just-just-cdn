// Cdn.h

#ifndef _PPBOX_CDN_CDN_H_
#define _PPBOX_CDN_CDN_H_

namespace ppbox
{
    namespace cdn
    {

        class Cdn
            : public ppbox::common::CommonModuleBase<Cdn>
        {
        public:
            Cdn(
                util::daemon::Daemon & daemon);

            ~Cdn();

        public:
            virtual boost::system::error_code startup();

            virtual void shutdown();
        };

    } // namespace cdn
} // namespace ppbox

#endif // _PPBOX_CDN_CDN_H_
