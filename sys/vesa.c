/* vesa.c
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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <kernel/vesa.h>

#include <stdio.h>

static struct vbe_info_block_t  VBE_INFO_BLOCK  = {0};
static struct mode_info_block_t MODE_INFO_BLOCK = {0};

uint32_t       x_resolution   = 0; /* Resolution of video mode used */
uint32_t       y_resolution   = 0; /* in width (x) and height (y) */
uint32_t       bytes_per_line = 0; /* Logical CRT scanline length */
uint32_t       current_bank   = 0; /* Current rw bank/window */
uint32_t       bank_shift     = 0; /* Bank granularity adjustment factor */
vbe_mode_num_t old_mode       = 0; /* Old/Previous video mode number */

volatile uint32_t *screen_ptr =
        ( uint32_t * )0xe0000000;  /* Pointer to video memory */
void (*bank_switch)(void);         /* Direct bank switching function */

int get_vbe_info(void)
{
    uint16_t result; /* AX register */

    /* Initialiase the outbound struct signature for VBE 2.0+ support */
    VBE_INFO_BLOCK.vbe_signature[0] = 'V';
    VBE_INFO_BLOCK.vbe_signature[1] = 'B';
    VBE_INFO_BLOCK.vbe_signature[2] = 'E';
    VBE_INFO_BLOCK.vbe_signature[3] = '2';

    __asm__ volatile("int $0x10"
                     : "=a"(result)                 /* AX = result */
                     : "a"(VESA_FUNC_GET_VBE_INFO), /* AX = 0x4F00 (get info
                                                       func code) */
                       "D"(&VBE_INFO_BLOCK) /* ES:DI = Pointer to info block */
                     : "memory");           /* Memory is modified */

    if (result != VBE_STATUS_FUNC_SUPPORTED)
        return 0;

    /* Verify VESA compatability with a valid return signature */
    if (VBE_INFO_BLOCK.vbe_signature[0] == 'V' &&
        VBE_INFO_BLOCK.vbe_signature[1] == 'E' &&
        VBE_INFO_BLOCK.vbe_signature[2] == 'S' &&
        VBE_INFO_BLOCK.vbe_signature[3] == 'A')
        return 1;

    return 0;
}

int get_vbe_mode_info(vbe_mode_num_t mode)
{
    uint16_t result; /* AX register */

    if (mode < 0x100)
        return 0;

    uint8_t zero = 0;
    memmove(( uint8_t * )&MODE_INFO_BLOCK, ( uint8_t * )&zero,
            sizeof(struct mode_info_block_t) * sizeof(uint8_t));

    __asm__ volatile(
            "int $0x10"
            : "=a"(result)                      /* AX = result */
            : "a"(VESA_FUNC_GET_VBE_MODE_INFO), /* AX = 0x4F01 (get mode info
                                                   func code) */
              "c"(mode),                        /* CX = mode number */
              "D"(&MODE_INFO_BLOCK) /* ES:DI = Pointer to mode block */
            : "memory");            /* Memory is modified */

    if (result != VBE_STATUS_FUNC_SUPPORTED)
        return 0;

    if (MODE_INFO_BLOCK.mode_attributes & 0x1 &&      /* Mode is supported */
        MODE_INFO_BLOCK.memory_model == mem_packed && /* 256 bit packed pixel */
        MODE_INFO_BLOCK.bits_per_pixel == 8 &&        /* 8 bit pixels */
        MODE_INFO_BLOCK.number_of_planes == 1)        /* Single plane */
        return 1;

    return 0;
}

int set_vbe_mode(vbe_mode_num_t mode)
{
    uint16_t result;
    __asm__ volatile("int $0x10"
                     : "=a"(result)                 /* AX = result */
                     : "a"(VESA_FUNC_SET_VBE_MODE), /* AX e 0x4F02 (set mode
                                                       func code) */
                       "b"(mode)                    /* BX = mode number */
                     : "memory");                   /* Memory is modified */
    if (result != VBE_STATUS_FUNC_SUPPORTED)
        return 0;
    return 1;
}

vbe_mode_num_t get_vbe_mode(void)
{
    vbe_mode_num_t mode;
    __asm__ volatile(
            "int $0x10"
            : "=b"(mode)                  /* BX = result */
            : "a"(VESA_FUNC_GET_VBE_MODE) /* AX e 0x4F03 (get mode func code) */
            : "memory");                  /* Memory is modified */
    return mode;
}

int set_vbe_bank(uint32_t bank)
{
    uint16_t result;

    if (bank == current_bank)
        return 1;

    current_bank   = bank;
    bank         <<= bank_shift;

    __asm__ volatile(
            "int $0x10"
            : "=a"(result)            /* AX = result */
            : "a"(VESA_FUNC_VBE_DWC), /* AX e 0x4F05 (set bank func code) */
              "b"(0x0000),            /* BX = window A */
              "d"(bank)               /* DX = bank in window granularity */
            : "memory");              /* Memory is modified */

    if (result != VBE_STATUS_FUNC_SUPPORTED)
        return 0;

    result = 0;

    __asm__ volatile(
            "int $0x10"
            : "=a"(result)            /* AX = result */
            : "a"(VESA_FUNC_VBE_DWC), /* AX e 0x4F05 (set bank func code) */
              "b"(0x0001),            /* BX = window B */
              "d"(bank)               /* DX = bank in window granularity */
            : "memory");              /* Memory is modified */

    if (result != VBE_STATUS_FUNC_SUPPORTED)
        return 0;

    return 1;
}

void put_pixel(uint32_t x, uint32_t y, uint32_t colour)
{
    uintptr_t addr = y * (bytes_per_line + x);
    set_vbe_bank(( uint32_t )(addr >> 16));
    *(( uint8_t * )screen_ptr + (addr & 0xFFFF)) = ( uint8_t )colour;
}

void line(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t colour)
{
    uint32_t d;             /* Decicision vaiable */
    uint32_t dx, dy;        /* Dx and Dy values for line */
    uint32_t Eincr, NEincr; /* Decision variable increments */
    uint32_t yincr;         /* Increment for y value */
    uint32_t t;             /* Counters, Swaps, etc. */

    dx = abs(x2 - x1);
    dy = abs(y2 - y1);
    if (dy <= dx) {
        /* Line with a slope -1 <= s <= 1
         *
         * Ensure that we are always scan:
         *      - converting the line from left to right
         *        to ensure that we produce the same line
         *        from P1 to P0 as the line from P0 to P1 */

        /* Swap X and Y values */
        if (x2 < x1) {
            t  = x2;
            x2 = x1;
            x1 = t;
            t  = y2;
            y2 = y1;
            y1 = t;
        }

        if (y2 > y1)
            yincr = 1;
        else
            yincr = -1;

        d      = (2 * dy) - dx;    /* Initialise deceision var */
        Eincr  = 2 * dy;           /* Increment to move the E pixel */
        NEincr = 2 * (dy - dx);    /* Incremeent to move the NE pixel */
        put_pixel(x1, y1, colour); /* Draw the first point at x1, y1 */

        /* Increment through rest of the points detereminend on the line */
        for (x1++; x1 <= x2; x1++) {
            if (d < 0)
                d += Eincr;            /* Choose eastern pixel */
            else {
                d  += NEincr;          /*  Choose the north eastern pixel */
                y1 += yincr;           /* (or south eastern for dx/dy < 0 ) */
            }
            put_pixel(x1, y1, colour); /* draw point */
        }
    } else {
        /* Line with a slope including vertical lines and others not covered
         *
         * Ensure that we are always scan: - converting the line from left to
         * right to ensure that we produce the same line from P1 to P0 as the
         * line from P0 to P1 */

        /* Swap X and Y values */
        if (y2 < y1) {
            t  = x2;
            x2 = x1;
            x1 = t;
            t  = y2;
            y2 = y1;
            y1 = t;
        }

        if (x2 > x1)
            yincr = 1;
        else
            yincr = -1;

        d      = (2 * dx) - dy;    /* Initialise deceision var */
        Eincr  = 2 * dx;           /* Increment to move the E pixel */
        NEincr = 2 * (dx - dy);    /* Incremeent to move the NE pixel */
        put_pixel(x1, y1, colour); /* Draw the first point at x1, y1 */

        /* Increment through rest of the points detereminend on the line */
        for (y1++; y1 <= y2; y1++) {
            if (d < 0)
                d += Eincr;            /* Choose eastern pixel */
            else {
                d  += NEincr;          /*  Choose the north eastern pixel */
                x1 += yincr;           /* (or south eastern for dx/dy < 0 ) */
            }
            put_pixel(x1, y1, colour); /* draw point */
        }
    }
}

/* Draw a simple moire pattern of lines on the display */
void draw_moire(void)
{
    uint32_t i;
    for (i = 0; i < x_resolution; i += 5) {
        line(x_resolution / 2, y_resolution / 2, i, 0, i % 0xFF);
        line(x_resolution / 2, y_resolution / 2, i, y_resolution,
             (i + 1) % 0xFF);
    }
    for (i = 0; i < y_resolution; i += 5) {
        line(x_resolution / 2, y_resolution / 2, 0, i, (i + 2) % 0xFF);
        line(x_resolution / 2, y_resolution / 2, x_resolution, i,
             (i + 3) % 0xFF);
    }
    line(0, 0, x_resolution - 1, 0, 15);
    line(0, 0, 0, y_resolution - 1, 15);
    line(x_resolution - 1, 0, x_resolution - 1, y_resolution - 1, 15);
    line(0, y_resolution - 1, x_resolution - 1, y_resolution - 1, 15);
}

void available_modes(void)
{
    uintptr_t p;
    if (!get_vbe_info()) {
        printf("[vbe] No VESA VBE detected\n");
        abort();
    }
    printf("[vbe] VESA VBE Version %d.%d detected (%s)\n\n",
           VBE_INFO_BLOCK.vbe_version >> 8, VBE_INFO_BLOCK.vbe_version & 0xF,
           ( char * )VBE_INFO_BLOCK.oem_product_name_ptr);
    printf("[vbe] Available 256 color video modes:\n");
    for (p = VBE_INFO_BLOCK.video_mode_ptr; p != ( unsigned )-1; ++p) {
        if (get_vbe_mode_info(( vbe_mode_num_t )p))
            printf("        - %4d x %4d %d bits per pixel\n",
                   MODE_INFO_BLOCK.x_resolution, MODE_INFO_BLOCK.y_resolution,
                   MODE_INFO_BLOCK.bits_per_pixel);
    }
    return;
}

/* Initialize the specified video mode. Determine a shift factor for adjusting
 * the Window granularity for bank switching. Much faster than a multiply */
void init_vbe(uint32_t x, uint32_t y, uintptr_t addr)
{
    uintptr_t p;
    if (!get_vbe_info()) {
        printf("[vbe] No VESA VBE detected\n");
        abort();
    }
    for (p = VBE_INFO_BLOCK.video_mode_ptr; p != ( unsigned )-1; ++p) {
        if (get_vbe_mode_info(( vbe_mode_num_t )p) &&
            MODE_INFO_BLOCK.x_resolution == x &&
            MODE_INFO_BLOCK.y_resolution == y) {
            x_resolution   = x;
            y_resolution   = y;
            bytes_per_line = MODE_INFO_BLOCK.bytes_per_scan_line;
            bank_shift     = 0;
            while (( uint32_t )(64 >> bank_shift) !=
                   MODE_INFO_BLOCK.win_granularity)
                bank_shift++;
            bank_switch  = ( void * )( uintptr_t )MODE_INFO_BLOCK.win_func_ptr;
            current_bank = -1;
            screen_ptr   = ( uint32_t * )addr;
            old_mode     = get_vbe_mode();
            set_vbe_mode(( vbe_mode_num_t )p);
            return;
        }
    }
    printf("[vbe] Valid video mode not found\n");
    abort();
}

int outp(uint16_t port, uint16_t data_byte)
{
    unsigned char value = ( unsigned char )(data_byte & 0xFF);
    __asm__ volatile("outb %b0, %w1" : : "a"(value), "d"(port));
    return data_byte;
}

unsigned short outpw(uint16_t port, uint16_t data_word)
{
    __asm__ volatile("outw %w0, %w1" : : "a"(data_word), "d"(port));
    return data_word;
}

/* Initialize Planar (Write mode 2) Should be Called from init_vbe */
void init_planar()
{
    outpw(0x3C4, 0x0F02);
    outpw(0x3CE, 0x0003);
    outpw(0x3CE, 0x0205);
    outpw(0x3CE, 0x0005);
}

/* Plot a pixel in Planar mode */
void put_pixel_p(uint32_t x, uint32_t y, uint32_t colour)
{
    uint8_t   dummy_read;
    uintptr_t addr = y * bytes_per_line + (x / 8);
    set_vbe_bank(( uint32_t )(addr >> 16));
    outp(0x3CE, 8);
    outp(0x3CF, 0x80 >> (x & 7));
    dummy_read = *(screen_ptr + (addr & 0xFFFF));
    *(( uint8_t * )screen_ptr + (addr & 0xFFFF)) = ( uint8_t )colour;
}

// vim: ft=c ts=4 sts=4 sw=4 et ai cin
