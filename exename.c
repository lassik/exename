// Copyright 2020 Lassi Kortela
// SPDX-License-Identifier: ISC

#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
static int once(char *buf, size_t cap)
{
    DWORD len = GetModuleFileNameA(NULL, buf, cap);
    if (!len) {
        return -1;
    }
    return len < cap - 1;
}
#endif

#if defined(__APPLE__) && defined(__MACH__)
#include <mach-o/dyld.h>
static int once(char *buf, size_t cap)
{
    uint32_t tmp = (uint32_t)cap;
    return _NSGetExecutablePath(buf, &tmp) == 0;
}
#endif

#if defined(__linux__) || defined(__CYGWIN__) || defined(__sun)
#include <unistd.h>
static int once(char *buf, size_t cap)
{
#ifdef __sun
    static const char procfs[] = "/proc/self/path/a.out";
#else
    static const char procfs[] = "/proc/self/exe";
#endif
    ssize_t len = readlink(procfs, buf, cap);
    if (len == (ssize_t)-1) {
        return -1;
    }
    if ((size_t)len < cap) {
        buf[len] = 0;
        return 1;
    }
    return 0;
}
#endif

#if defined(__FreeBSD__) || defined(__DragonFly__) || defined(__NetBSD__)
#include <errno.h>
#include <sys/sysctl.h>
static int once(char *buf, size_t cap)
{
#ifdef __NetBSD__
    int mib[4] = { CTL_KERN, KERN_PROC_ARGS, -1, KERN_PROC_PATHNAME };
#else
    int mib[4] = { CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1 };
#endif
    size_t len = cap;
    if (sysctl(mib, 4, buf, &len, 0, 0) == 0) {
        return 1;
    }
    return (errno == ENOMEM) ? 0 : -1;
}
#endif

#include "exename.h"

char *exename(void)
{
    char *buf;
    char *newbuf;
    size_t cap;
    int ret;

    buf = NULL;
    for (cap = 64; cap <= 16384; cap *= 2) {
        if (!(newbuf = realloc(buf, cap))) {
            break;
        }
        buf = newbuf;
        ret = once(buf, cap);
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
