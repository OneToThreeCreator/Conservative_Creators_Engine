/*
    Conservative Creator's Engine - open source engine for making games.
    Copyright (C) 2020-2022 Andrey Gaivoronskiy

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
*/

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
#define _POSIX_C_SOURCE 200109L
#define _XOPEN_SOURCE 500L

#elif defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__) || \
      defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)

#define WINDOWS_SYSTEM

#else

#error "Target platform is not supported"

#endif

#endif // PLATFORMS_H
