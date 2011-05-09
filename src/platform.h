/* Copyright (c) 2011 Vector Fabrics B.V. All rights reserved.
 *
 * This file contains proprietary and confidential information of Vector
 * Fabrics and all use (including distribution) is subject to the conditions of
 * the license agreement between you and Vector Fabrics. Without such a license
 * agreement in place, no usage or distribution rights are granted by Vector
 * Fabrics.
 */

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
