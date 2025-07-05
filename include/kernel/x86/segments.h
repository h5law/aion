/* segments.h
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

#ifndef _KERNEL_SEGMENTS_H
#define _KERNEL_SEGMENTS_H

#include <stdint.h>

/* clang-format off */
typedef struct {
    /* [15 : 3] - Index (in specified table)
     * [2] - Table indicator (0 GDT, 1 LDT)
     * [1 : 0] - Request Privilege Level ( 3 - least privileged, 0 - most privileged */
    uint16_t selector;

    /* [31 : 0] - Offset of segment byte relative to the linear base address */
    uint32_t offset;
} logical_address_t;
/* clang-format on */

#define SEL_INDEX(selector) ( uint16_t )selector & 0xFF80

#define SEL_TABLE(selector) ( uint16_t )selector & 0x4

#define SEL_RPL(selector)   ( uint16_t )selector & 0x3

/* clang-format off */
typedef struct {
    /* [15 : 0] - Segment Limit (offset range) */
    uint16_t limit;

    /* [15 : 0] - Segment Base Address (linear address lower bits 15 : 0) */
    uint16_t base_addr;

    /* [7 : 0] - Base Address expanded lower bits (linear address 23 : 16) */
    uint8_t base_lower;

    /* [7] - Segment Present (0 not in memory, 1 in memory)
     * [6 : 5] - Descriptor Privilege Level (3 - least privileged, 0 - most privileged)
     * [4] - Descriptor type (0 system, 1 code/data)
     * [3 : 0] - Type
     *  system 32-bit mode              | IA-32e mode
     *  - 0000 reserved                 | reserved
     *  - 0001 16-bit TSS (available)   | reserved
     *  - 0010 LDT                      | LDT
     *  - 0011 16-bit TSS (busy)        | reserved
     *  - 0100 16-bit Call Gate         | reserved
     *  - 0101 Task Gate                | reserved
     *  - 0110 16-bit Interrupt Gate    | reserved
     *  - 0111 16-bit Trap Gate         | reserved
     *  - 1000 reserved                 | reserved
     *  - 1001 32-bit TSS (available)   | 64-bit TSS (available)
     *  - 1010 reserved                 | reserved
     *  - 1011 32-bit TSS (busy)        | 64-bit TSS (busy)
     *  - 1100 32-bit Call Gate         | 64-bit Call Gate
     *  - 1101 reserved                 | reserved
     *  - 1110 32-bit Interrupt Gate    | 64-bit Interrupt Gate
     *  - 1111 32-bit Trap Gate         | 64-bit Trap Gate
     *  code/data
     *  - 0000 ro
     *  - 0001 ro accessed
     *  - 0010 rw
     *  - 0011 rw accessed
     *  - 0100 ro expand-down
     *  - 0101 ro expand-down accessed
     *  - 0110 rw expand-down
     *  - 0111 rw expand-down accessed
     *  - 1000 xo
     *  - 1001 xo accessed
     *  - 1010 rx
     *  - 1011 rx accessed
     *  - 1100 xo conforming
     *  - 1101 xo conforming accessed
     *  - 1110 rx conforming
     *  - 1111 rx conforming accessed
     * */
    uint8_t      access;

    /* [3 : 0] - Limit upper 4 bits (19 : 16) */
    unsigned int limit_upper : 4;

    /* [3] - Granularity (0 1B-1MB byte-incr segments, 1 4KB-4GB 4KB-incr segments)
     * [2] - Default Op-Size (0 16-bit segment, 1 32-bit segment)
     * [1] - 64-bit code segment (IA-32e mode only)
     * [0] - Availability for use by system software */
    unsigned int flags       : 4;

    /* [7 : 0] - Base Address expanded upper bits (linear address 31 : 24) */
    uint8_t      base_upper;
} segment_descriptor_t;
/* clang-format on */

void convert_descriptor(uint32_t lower, uint32_t upper,
                        segment_descriptor_t *descriptor);

#define DESCRIPTOR(word, desc_ptr)                                             \
    convert_descriptor(( uint64_t )word & 0x00000000FFFFFFFF,                  \
                       ( uint64_t )word & 0xFFFFFFFF00000000,                  \
                       ( segment_descriptor_t * )desc_ptr);

#define SEG_BASE_ADDR(desc_ptr)                                                \
    (( uint32_t )(( segment_descriptor_t * )desc_ptr->base_addr) |             \
     (( uint32_t )(( segment_descriptor_t * )desc_ptr->base_lower) << 16) |    \
     (( uint32_t )(( segment_descriptor_t * )desc_ptr->base_upper) << 24))

#define SEG_LIMIT(desc_ptr)                                                    \
    (( uint32_t )(( segment_descriptor_t * )desc_ptr->limit) |                 \
     (( uint32_t )(( segment_descriptor_t * )desc_ptr->limit_upper) << 16))

#define SEG_TYPE(desc_ptr)                                                     \
    ( segment_descriptor_t * )desc_ptr->access & 0b00001111

#define SEG_DESCRIPTOR(desc_ptr)                                               \
    ( segment_descriptor_t * )desc_ptr->access & 0b00010000

#define SEG_PRIVILEGE(desc_ptr)                                                \
    ( segment_descriptor_t * )desc_ptr->access & 0b01100000

#define SEG_PRESENT(desc_ptr)                                                  \
    ( segment_descriptor_t * )desc_ptr->access & 0b10000000

#define SEG_GRANULARITY(desc_ptr)                                              \
    ( segment_descriptor_t * )desc_ptr->flags & 0b1000

#define SEG_OP_SIZE(desc_ptr) ( segment_descriptor_t * )desc_ptr->flags & 0b0100

#define SEG_64_BIT(desc_ptr)  ( segment_descriptor_t * )desc_ptr->flags & 0b0010

#define SEG_SOFT_AVAIL(desc_ptr)                                               \
    ( segment_descriptor_t * )desc_ptr->flags & 0b0001

#endif /* _KERNEL_SEGMENTS_H */

// vim: ft=c ts=4 sts=4 sw=4 et ai cin
