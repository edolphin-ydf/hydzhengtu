#ifndef __HIREDIS_FMACRO_H
#define __HIREDIS_FMACRO_H

#if !defined(_BSD_SOURCE)
#define _BSD_SOURCE
#endif

#if defined(__sun__)
#define _POSIX_C_SOURCE 200112L
#elif defined(__linux__)
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE
#endif

#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

#ifdef _WIN32
#define off off_t
#endif

#endif
