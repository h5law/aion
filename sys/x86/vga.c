/* vga.c
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

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/vga.h>

static const size_t       VGA_WIDTH  = 80;
static const size_t       VGA_HEIGHT = 25;
static volatile uint16_t *VGA_MEMORY = ( uint16_t * )0xB8000;

static volatile uint16_t *vga_buffer;
static size_t             vga_row;
static size_t             vga_column;
static uint8_t            vga_colour;

void vga_init(void)
{
    vga_row    = 0;
    vga_column = 0;
    vga_colour = vga_entry_colour(VGA_COLOUR_LIGHT_GRAY, VGA_COLOUR_BLACK);
    vga_buffer = VGA_MEMORY;
    for (size_t y = 0; y < VGA_HEIGHT; ++y) {
        for (size_t x = 0; x < VGA_HEIGHT; ++x) {
            const size_t idx = (y * VGA_WIDTH) + x;
            vga_buffer[idx]  = vga_entry(' ', vga_colour);
        }
    }
}

void vga_setcolour(uint8_t fg, uint8_t bg)
{
    vga_colour = vga_entry_colour(fg, bg);
}

void vga_putentry(unsigned char c, uint8_t colour, size_t x, size_t y)
{
    const size_t idx = (y * VGA_WIDTH) + x;
    vga_buffer[idx]  = vga_entry(c, colour);
}

void vga_scroll(int line)
{
    char               c;
    volatile uint16_t *ptr;

    for (ptr = VGA_MEMORY + (line - 1 * VGA_WIDTH);
         ptr < VGA_MEMORY + (line - 1 * (VGA_WIDTH * 2)); ++ptr) {
        c                  = *ptr;
        *(ptr - VGA_WIDTH) = c;
    }
}

void vga_delete_line(int line)
{
    volatile uint16_t *ptr;
    for (size_t x = 0; x < VGA_WIDTH; ++x) {
        ptr  = VGA_MEMORY + (((line - 1) * VGA_WIDTH) + x);
        *ptr = 0;
    }
}

void vga_delete_last_line(void) { vga_delete_line(VGA_HEIGHT - 1); }

void vga_putchar(char c)
{
    size_t line;

    if (c == '\n') {
        vga_column = 0;
        if (++vga_row >= VGA_HEIGHT) {
            for (line = 1; line <= VGA_HEIGHT - 1; ++line)
                vga_scroll(line);
            vga_delete_last_line();
            vga_row = VGA_HEIGHT - 1;
        }
        return;
    }
    vga_putentry(( unsigned char )c, vga_colour, vga_column, vga_row);
    if (++vga_column == VGA_WIDTH) {
        vga_column = 0;
        if (++vga_row == VGA_HEIGHT) {
            for (line = 1; line <= VGA_HEIGHT - 1; ++line)
                vga_scroll(line);
            vga_delete_last_line();
            vga_row = VGA_HEIGHT - 1;
        }
    }
}

void vga_write(const char *data, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        vga_putchar(data[i]);
}

void vga_writes(const char *data) { vga_write(data, strlen(data)); }

// vim: ft=c ts=4 sts=4 sw=4 et ai cin
