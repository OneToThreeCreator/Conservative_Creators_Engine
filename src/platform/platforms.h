#ifndef PLATFORMS_H
#define PLATFORMS_H

#if defined(unix) || defined(__unix__) || defined(__unix) || \
   (defined(__APPLE__) && defined(__MACH__)) || \
    defined(linux) || defined(__linux__) || defined(__linux) || \
    defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__bsdi__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__) || \
    defined(__GNU__) || defined(__gnu_hurd__) || \
    defined(sun) || defined(__sun) || defined (sinux) || defined(__minix)

#define POSIX_SYSTEM
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500L

#elif defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__) || \
      defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)

#define WINDOWS_SYSTEM

#else

#error "Target platform is not supported"

#endif

#endif // PLATFORMS_H
