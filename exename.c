// Copyright 2020 Lassi Kortela
// SPDX-License-Identifier: ISC

#ifdef _WIN32
#include <windows.h>
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__OpenBSD__)   \
    || defined(__NetBSD__)
#include <sys/types.h>

#include <sys/sysctl.h>
#endif

#include <errno.h>
#include <stdlib.h>

#include "exename.h"

#ifdef _WIN32
static int once(char *buf, size_t n)
{
    DWORD nr = GetModuleFileNameA(NULL, buf, n);
    if (!nr) {
        return -1;
    }
    return nr < n - 1;
}
#endif

#if defined(__APPLE__) && defined(__MACH__)
static int once(char *buf, size_t n)
{
    uint32_t tmp = (uint32_t)n;
    return _NSGetExecutablePath(buf, &tmp) == 0;
}
#endif

#ifdef __linux__
static int once(char *buf, size_t n)
{
    ssize_t nr = readlink("/proc/self/exe", buf, n);
    if (nr == (ssize_t)-1) {
        return -1;
    }
    return (size_t)nr < n;
}
#endif

#if defined(__FreeBSD__) || defined(__DragonFly__)
static int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
#endif
#ifdef __NetBSD__
static int mib[4] = { CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME };
#endif

#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__)
static int once(char *buf, size_t n)
{
    size_t nr = n;
    if (sysctl(mib, 4, buf, &nr, 0, 0) == 0) {
        return 1;
    }
    return (errno == ENOMEM) ? 0 : -1;
}
#endif

#ifdef __OpenBSD__
#endif

#ifdef __sun
#endif

#ifdef __HAIKU__
#endif

char *exename(void)
{
    char *buf;
    char *newbuf;
    size_t n;
    int ret;

    buf = NULL;
    for (n = 64; n <= 16384; n *= 2) {
        if (!(newbuf = realloc(buf, n))) {
            break;
        }
        buf = newbuf;
        ret = once(buf, n);
        if (ret == 1) {
            return buf;
        }
        if (ret) {
            break;
        }
    }
    free(buf);
    return NULL;
}
