#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _POSIX_SOURCE
#include "threading_sync_defs_posix.h"
#elif defined (_WIN32)
#include "threading_sync_defs_win.h"
#else
#error("unsupported platform")
#endif /* _POSIX_SOURCE / _WIN32 */

#endif /* PLATFORM_H */
