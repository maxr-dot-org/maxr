
#if HAVE_AUTOVERSION_H
# include "autoversion.h" //include autoversion created by buildinfo.sh or cmake
#else // We have no autoversion => take care of these manually!
//default path to data dir only used on linux/other
# define BUILD_DATADIR "/usr/share/maxr"
// Builddate: Mmm DD YYYY HH:MM:SS
# define MAX_BUILD_DATE ((std::string)__DATE__ + " " + __TIME__)
# ifdef RELEASE
#  define PACKAGE_REV "Releaseversion"
# else
#    define PACKAGE_REV "GIT Hash unknown"
# endif
#endif

#if HAVE_CONFIG_H
# include "config.h"
#else // We have no config.h => take care of these manually
# define PACKAGE_VERSION "0.2.8"
# define PACKAGE_NAME "M.A.X.R."
#endif
