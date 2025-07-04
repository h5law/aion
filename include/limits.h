/* limits.h
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

#ifndef _LIMITS_H
#define _LIMITS_H

/* Architecture and word size detection */
#if defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 8
#define WORDSIZE 64
#elif defined(__SIZEOF_POINTER__) && __SIZEOF_POINTER__ == 4
#define WORDSIZE 32
#elif defined(_WIN64) || defined(__x86_64__) || defined(__amd64__) ||          \
        defined(__aarch64__)
#define WORDSIZE 64
#elif defined(_WIN32) || defined(__i386__) || defined(__arm__)
#define WORDSIZE 32
#else
#define WORDSIZE 32 /* Default assumption */
#endif

/* Character type limits */
#define CHAR_BIT  8
#define SCHAR_MIN (-128)
#define SCHAR_MAX (127)
#define UCHAR_MAX (255)

#if '\xff' == -1
#define CHAR_MIN SCHAR_MIN
#define CHAR_MAX SCHAR_MAX
#else
#define CHAR_MIN 0
#define CHAR_MAX UCHAR_MAX
#endif

/* Short type limits (always 16-bit) */
#define SHRT_MIN  (-32768)
#define SHRT_MAX  (32767)
#define USHRT_MAX (65535)

/* Int type limits (typically 32-bit) */
#if WORDSIZE == 64
#define INT_MIN  (-2147483648)
#define INT_MAX  (2147483647)
#define UINT_MAX (4294967295U)
#else
#define INT_MIN  (-2147483648)
#define INT_MAX  (2147483647)
#define UINT_MAX (4294967295U)
#endif

/* Long type limits (word-size dependent) */
#if WORDSIZE == 64
#define LONG_MIN  (-9223372036854775808L)
#define LONG_MAX  (9223372036854775807L)
#define ULONG_MAX (18446744073709551615UL)
#else
#define LONG_MIN  (-2147483648L)
#define LONG_MAX  (2147483647L)
#define ULONG_MAX (4294967295UL)
#endif

/* Long long type limits (always 64-bit when available) */
#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
#define LLONG_MIN  (-9223372036854775808LL)
#define LLONG_MAX  (9223372036854775807LL)
#define ULLONG_MAX (18446744073709551615ULL)
#endif

/* Fixed-width integer limits (derived from basic type limits) */
#define INT8_MIN   SCHAR_MIN
#define INT8_MAX   SCHAR_MAX
#define UINT8_MAX  UCHAR_MAX

#define INT16_MIN  SHRT_MIN
#define INT16_MAX  SHRT_MAX
#define UINT16_MAX USHRT_MAX

#define INT32_MIN  INT_MIN
#define INT32_MAX  INT_MAX
#define UINT32_MAX UINT_MAX

#if WORDSIZE == 64
#define INT64_MIN  LONG_MIN
#define INT64_MAX  LONG_MAX
#define UINT64_MAX ULONG_MAX
#else
#define INT64_MIN  LLONG_MIN
#define INT64_MAX  LLONG_MAX
#define UINT64_MAX ULLONG_MAX
#endif

/* Pointer-sized integer types */
#if WORDSIZE == 64
#define INTPTR_MIN  INT64_MIN
#define INTPTR_MAX  INT64_MAX
#define UINTPTR_MAX UINT64_MAX
#else
#define INTPTR_MIN  INT32_MIN
#define INTPTR_MAX  INT32_MAX
#define UINTPTR_MAX UINT32_MAX
#endif

/* Maximum-width integer types */
#if defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
#define INTMAX_MIN  INT64_MIN
#define INTMAX_MAX  INT64_MAX
#define UINTMAX_MAX UINT64_MAX
#else
#define INTMAX_MIN  LONG_MIN
#define INTMAX_MAX  LONG_MAX
#define UINTMAX_MAX ULONG_MAX
#endif

#endif /* _LIMITS_H */

// vim: ft=c ts=4 sts=4 sw=4 et ai cin
