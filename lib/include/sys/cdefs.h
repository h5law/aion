/* cdefs.h
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

#ifndef _SYS_CDEFS_H
#define _SYS_CDEFS_H

#ifndef _u
#ifdef __ASSEMBLER__
#define _u(x) x
#else
#define _u(x) x##u
#endif
#endif

#ifndef __has_attribute
#define __has_attribute(x) 0
#endif
#ifndef __has_extension
#define __has_extension __has_feature
#endif
#ifndef __has_feature
#define __has_feature(x) 0
#endif
#ifndef __has_include
#define __has_include(x) 0
#endif
#ifndef __has_builtin
#define __has_builtin(x) 0
#endif

#if __has_attribute(__noreturn__)
#define __noreturn __attribute__((__noreturn__))
#else
#define __noreturn
#endif

#if __has_attribute(__always_inline__)
#define __always_inline __inline __attribute__((__always_inline__))
#else
#define __always_inline
#endif

#if __has_attribute(__disable_sanitizer_instrumentation__)
#define __disable_sanitizer_instrumentation                                    \
    __attribute__((__disable_sanitizer_instrumentation__))
#else
#define __disable_sanitizer_instrumentation
#endif

#if __has_attribute(__no_sanitize_address__)
#define __no_sanitize_address __attribute__((__no_sanitize_address__))
#else
#define __no_sanitize_address
#endif

#if __has_attribute(__no_sanitize__)
#define __no_sanitize(s) __attribute__((__no_sanitize__(s)))
#else
#define __no_sanitize(s)
#endif

#if __has_attribute(__no_sanitize_undefined__)
#define __no_sanitize_undefined __attribute__((__no_sanitize_undefined__))
#else
#define __no_sanitize_undefined
#endif

#define __disable_sanitizer                                                    \
    __no_sanitize_address __no_sanitize_undefined                              \
            __disable_sanitizer_instrumentation __no_sanitize("undefined")

#if __has_attribute(__naked__)
#define __naked __attribute__((__naked__))
#else
#define __naked
#endif

#if __has_attribute(__noinline__)
#define __noinline __attribute__((__noinline__))
#else
#define __noinline
#endif

#define __CONCAT1(x, y) x##y
#define __CONCAT(x, y)  __CONCAT1(x, y)
#define __STRING(x)     #x          /* stringify without expanding x */
#define __XSTRING(x)    __STRING(x) /* expand x, then stringify */

#if defined(__cplusplus)
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS   }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif

#define __weak_symbol       __attribute__((__weak__))
#define __dead2             __attribute__((__noreturn__))
#define __pure2             __attribute__((__const__))
#define __unused            __attribute__((__unused__))
#define __used              __attribute__((__used__))
#define __deprecated        __attribute__((__deprecated__))
#define __deprecated1(msg)  __attribute__((__deprecated__(msg)))
#define __packed            __attribute__((__packed__))
#define __aligned(x)        __attribute__((__aligned__(x)))
#define __section(x)        __attribute__((__section__(x)))
#define __writeonly         __unused
#define __alloc_size(x)     __attribute__((__alloc_size__(x)))
#define __alloc_size2(n, x) __attribute__((__alloc_size__(n, x)))
#define __alloc_align(x)    __attribute__((__alloc_align__(x)))

#endif /* _SYS_CDEFS_H */

// vim: ft=c ts=4 sts=4 sw=4 et ai cin
