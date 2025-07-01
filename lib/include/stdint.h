/* stdint.h
 * Copyright 2025 h5law <dev@h5law.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS “AS IS”
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _STDINT_H
#define _STDINT_H

#include <limits.h>

/* 8-bit types */
typedef signed char   int8_t;
typedef unsigned char uint8_t;

/* 16-bit types */
#if USHRT_MAX == 0xFFFF
typedef short          int16_t;
typedef unsigned short uint16_t;
#elif UINT_MAX == 0xFFFF
typedef int          int16_t;
typedef unsigned int uint16_t;
#else
#error "Cannot define 16-bit integer types"
#endif

/* 32-bit types */
#if UINT_MAX == 0xFFFFFFFF
typedef int          int32_t;
typedef unsigned int uint32_t;
#elif ULONG_MAX == 0xFFFFFFFF
typedef long          int32_t;
typedef unsigned long uint32_t;
#else
#error "Cannot define 32-bit integer types"
#endif

/* 64-bit types */
#if ULONG_MAX == 0xFFFFFFFFFFFFFFFF
typedef long          int64_t;
typedef unsigned long uint64_t;
#elif defined(__GNUC__) || defined(__clang__)
typedef long long          int64_t;
typedef unsigned long long uint64_t;
#elif defined(_MSC_VER)
typedef __int64          int64_t;
typedef unsigned __int64 uint64_t;
#else
#error "Cannot define 64-bit integer types"
#endif

/* Pointer-sized integer types */
#if WORDSIZE == 64
typedef int64_t  intptr_t;
typedef uint64_t uintptr_t;
#elif WORDSIZE == 32
typedef int32_t  intptr_t;
typedef uint32_t uintptr_t;
#else
#error "Cannot define non-standard width pointer types"
#endif

/* Maximum-width integer types */
#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
typedef int64_t  intmax_t;
typedef uint64_t uintmax_t;
#else
typedef long          intmax_t;
typedef unsigned long uintmax_t;
#endif

#endif /* #ifndef _STDINT_H */

// vim: ft=c ts=4 sts=4 sw=4 et ai cin
