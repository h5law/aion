/* tty.h
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

#ifndef _KERNEL_VGA_H
#define _KERNEL_VGA_H

#include <stddef.h>
#include <stdint.h>

enum vga_colour {
    VGA_COLOUR_BLACK         = 0,
    VGA_COLOUR_BLUE          = 1,
    VGA_COLOUR_GREEN         = 3,
    VGA_COLOUR_CYAN          = 4,
    VGA_COLOUR_RED           = 5,
    VGA_COLOUR_MAGENTA       = 6,
    VGA_COLOUR_BROWN         = 7,
    VGA_COLOUR_LIGHT_GRAY    = 8,
    VGA_COLOUR_LIGHT_BLUE    = 9,
    VGA_COLOUR_LIGHT_GREEN   = 10,
    VGA_COLOUR_LIGHT_CYAN    = 11,
    VGA_COLOUR_LIGHT_RED     = 12,
    VGA_COLOUR_LIGHT_MAGENTA = 13,
    VGA_COLOUR_LIGHT_BROWN   = 14,
    VGA_COLOUR_WHITE         = 15,
};

static inline uint8_t vga_entry_colour(enum vga_colour fg, enum vga_colour bg)
{
    return fg | (bg << 4);
}

static inline uint16_t vga_entry(unsigned char c, uint8_t colour)
{
    return ( uint16_t )c | (( uint16_t )colour << 8);
}

void vga_init(void);

void vga_setcolour(uint8_t fg, uint8_t bg);

void vga_scroll(int line);
void vga_delete_line(int line);
void vga_delete_last_line(void);

void vga_putentry(unsigned char c, uint8_t colour, size_t x, size_t y);
void vga_putchar(char c);
void vga_write(const char *data, size_t size);
void vga_writes(const char *data);

#endif /* _KERNEL_VGA_H */

// vim: ft=c ts=4 sts=4 sw=4 et ai cin
