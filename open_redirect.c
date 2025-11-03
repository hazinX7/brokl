#define _GNU_SOURCE
#include <dlfcn.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

static const char *target = "/etc/update_fw_tail";
static const char *redirect = "./update_fw_tail";

int open(const char *pathname, int flags, ...)
{
    static int (*real_open)(const char *, int, ...) = NULL;
    if (!real_open) real_open = dlsym(RTLD_NEXT, "open");

    if (pathname && strcmp(pathname, target) == 0)
        pathname = redirect;

    va_list ap;
    va_start(ap, flags);
    int fd;
    if (flags & O_CREAT)
    {
        mode_t mode = va_arg(ap, mode_t);
        fd = real_open(pathname, flags, mode);
    }
    else
    {
        fd = real_open(pathname, flags);
    }
    va_end(ap);
    return fd;
}

int openat(int dirfd, const char *pathname, int flags, ...)
{
    static int (*real_openat)(int, const char *, int, ...) = NULL;
    if (!real_openat) real_openat = dlsym(RTLD_NEXT, "openat");

    if (pathname && strcmp(pathname, target) == 0)
        pathname = redirect;

    va_list ap;
    va_start(ap, flags);
    int fd;
    if (flags & O_CREAT)
    {
        mode_t mode = va_arg(ap, mode_t);
        fd = real_openat(dirfd, pathname, flags, mode);
    }
    else
    {
        fd = real_openat(dirfd, pathname, flags);
    }
    va_end(ap);
    return fd;
}
