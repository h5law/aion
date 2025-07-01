/* vesa.h
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

#ifndef _VESA_H
#define _VESA_H

#include <stdint.h>

#define VBE_RES_SUCCESS             0x004F /* AX value upon successful return */

/*****************************************************************************/
/*                              Function Codes                               */
/*****************************************************************************/

/* Get VBE information function
 * Input:   AX    = 0x4F00 - function code
 *          ES:DI = Segment:Offset to where vbe_info_t can be found
 * Output:  AX    = 0x004F (success) indicates ES:DI has valid information
 *                = 0xXXXX (failure) vbe information request failed bios error
 *                                   ES:DI may have invalid or corrupted data */
#define VESA_FUNC_GET_VBE_INFO      0x4F00

/* Get VBE mode information function
 * Input:   AX    = 0x4F01 - function code
 *          CV    = VESA mode number from video modes array
 *          ES:DI = Segment:Offset pointer to where vbe_mode_info_t can be found
 * Output:  AX    = 0x004F (success) indicates ES:DI has valid information
 *                = 0xXXXX (failure) mode (CV) is unsupported or bios error
 *                                   ES:DI may have invalid or corrupted data */
#define VESA_FUNC_GET_VBE_MODE_INFO 0x4F01

/* Set VBE mode function
 * Input:   AX    = 0x4F02 - function code
 *          BX    = [15] DM Bit - 1 BIOS doesn't clear screen (AVOID)
 *                                0 clears the screen
 *                  [14] LFB Bit - 1 enable linear frame buffer
 *                                 0 use bank switching`
 *                  [13-0] Mode number
 * Output:  AX    = 0x004F (success) the VBE mode has been set successfully
 *                = 0xXXXX (failure) indicates errors such as:
 *                                        - BIOS errors
 *                                        - Too little video memory
 *                                        - Unsupported VBE mode
 *                                        - Mode doesnt support LFB
 *                                        - ... */
#define VESA_FUNC_SET_VBE_MODE      0x4F02

/* Get VBE current mode function
 * Input:   AX    = 0x4F03 - function code
 * Output:  AX    = 0x004F (success) indicates ES:DI has valid information
 *                = 0xXXXX (failure) indicates errors such as:
 *                                       - BIOS errors
 *                                       - System not in VBE mode
 *                                       - ...
 *          BX    = [15] DM Bit - 1 BIOS did not clear screen
 *                                0 video memory was cleared when VBE mode set
 *                  [14] LFB Bit - 1 linear frame buffer is enabled
 *                                 0 bank switching`mode in use
 *                  [13-0] Mode number */
#define VESA_FUNC_GET_VBE_MODE      0x4F03

/* DEPRECATED : Display Window Control function
 * Input:   AX    = 0x4F05 - function code
 *          BH    = 0x00 - Set bank
 *                  0x01 - Get bank
 *          BL    = 0x00 - Window A
 *                  0x01 - Window B
 *          DX    = 0xXXXX - Bank nuber in window granularity units
 * Output:  AX    = 0x004F (success) indicates the operation was a success
 *                = 0xXXXX (failure) indicates errors such as:
 *                                       - BIOS errors
 *                                       - Unusable bank
 *                                       - Unpresent bank
 *                                       - Not in bank switching mode
 *                                       - ... */
#define VESA_FUNC_VBE_DWC           0x4F05

/* Get VBE 2.0+ Protected Mode Interface function
 * Input:   AX    = 0x4F0A - function code
 *          BL    = 0x00
 * Output:  AX    = 0x004F (success) success and ES:DI contains the interface
 *                = 0xXXXX (failure) indicates errors such as:
 *                                       - BIOS errors
 *                                       - Protected Mode Interface unsupported
 *                                       - VBE Version less than 2.0
 *                                       - ...
 *         ES:DI = Segment:Offset pointer to the vbe2_pmi_table_t structure
 *         CX    = Length of the table including code in bytes for copying */
#define VESA_FUNC_GET_VBE_PMI       0x4F0A

/*****************************************************************************/
/*                            VBE Data Structs */
/*****************************************************************************/

/* VBE General structure for getting and receiving VBE generic information */
struct vbe_info_t {
    /* Signature signalling software and hardware support */
    char signature[4]; /* OUT: "VESA" - valid VBE software support
                        * IN:  "XXXX" - VESA BIOS Extensions unsupported
                        *      "VESA" - hardware supports VBE 1.0+ (256 bits)
                        *      "VBE2" - hardware supports VBE 2.0+ (512 bits) */

    /* VBE Version: 0x0100 - v1.0
     *              0x0101 - v1.1
     *              0x0102 - v1.2
     *              0x0200 - v2.0
     *              0x0300 - v3.0 */
    uint16_t version; /* [15-8] Major version
                       * [7-0]  Minor version */

    /* Offset pointer into the segment for the OEM */
    uint32_t oem;

    /* Card capabilities bitfield */
    uint32_t capabilities;

    /* Offset pointer into the segment, listing supported video modes.
     * Each mode is represented as a 16-bit word terminated with 0xFFFF
     * Modes: 0x0103, 0x0115, 0x0118 == 03 01 15 01 18 01 FF FF */
    uint32_t video_modes; /* [31-16] Segment
                           * [15-0]  Offset */

    /* Amount of video memory available in 64KiB blocks, x64 for KiB */
    uint16_t video_memory;

    /* Software revision number */
    uint16_t software_rev;

    /* Offset pointer into the segment for the card vendor string */
    uint32_t vendor; /* [31-16] Segment
                      * [15-0]  Offset */

    /* Offset pointer into the segment for the card model string */
    uint32_t product_name; /* [31-16] Segment
                            * [15-0]  Offset */

    /* Offset pointer into the segment for the product's revision number */
    uint32_t product_rev; /* [31-16] Segment
                           * [15-0]  Offset */

    /* 222 bytes reserved for future expansions of the info structure */
    char reserved[222];

    /* OEM BIOS custom store for data strings and values */
    char oem_data[256];
} __attribute__((packed));

/* VBE Mode Information structure for getting and receiving mode info */
struct vbe_mode_info_t {
    /* SEMI-DEPRECATED */
    uint16_t attributes; /* [7] the mode supports linear frame buffers (0x80) */

    /* DEPRECATED */
    uint8_t window_a;

    /* DEPRECATED */
    uint8_t window_b;

    /* DEPRECATED */
    uint16_t granularity; /* Used to calculate bank numbers */

    /* Window size supported bu the VBE mode queried */
    uint16_t window_size; /* [15-0]  */

    uint16_t segment_a;

    uint16_t segment_b;

    /* DEPRECATED */
    uint32_t win_func_ptr; // deprecated; used to switch banks from protected
                           // mode without returning to real mode
    uint16_t pitch;        // number of bytes per horizontal line

    /* Width of the mode being queried for usage */
    uint16_t width; /* [15-0] number of pixels */

    /* Height of the mode being queried for usage */
    uint16_t height; /* [15-0] number of pixels */

    /* UNUSED */
    uint8_t w_char;

    /* UNUSED */
    uint8_t y_char;

    /* UNUSED */
    uint8_t planes;

    /* Bits-per-pixel of the mode being queried for usage */
    uint8_t bpp; /* [7-0] number of pixels */

    /* DEPRECATED */
    uint8_t banks; /* total number of banks in this mode */

    uint8_t memory_model;

    /* DEPRECATED */
    uint8_t bank_size; // Size of a bank (usually 64KiB sometimes 16KiB) */

    uint8_t image_pages;

    uint8_t reserved0;

    uint8_t red_mask;

    uint8_t red_position;

    uint8_t green_mask;

    uint8_t green_position;

    uint8_t blue_mask;

    uint8_t blue_position;

    uint8_t reserved_mask;

    uint8_t reserved_position;

    uint8_t direct_color_attributes;

    /* Physical address of the linear frame buffer in memory */
    uint32_t framebuffer; /* [31-0] Write address to draw to the screen
                           * When using paging ensure within virtual space */

    /* Offset from the frame buffer address which points to offscreen memory */
    uint32_t off_screen_mem_offset; /* [31-0] Offset from frame buffer*/

    /* Size of offscreen memory in the frame buffer */
    uint16_t off_screen_mem_size; /* [15-0] Size of memory not displayed */

    /* 206 bytes reserved for future expansions of the mode info structure */
    uint8_t reserved[206];
} __attribute__((packed));

/* VBE 2.0+ Protected Mode Interface detailing function pointers that can be
 * used in Protected Mode without switching back to Real Mode */
struct vbe2_pmi_table_t {
    /* Offset into table for protected mode code for function 0x4F05
     * Switch Banks/Window function call */
    uint16_t set_bank_win;

    /* Offset into table for protected mode code for function 0x4F07
     * Set display start function call */
    uint16_t set_display_start;

    /* Offset into table for protected mode code for function 0x4F09
     * Set the primary pallette data function call*/
    uint16_t set_pallette;
} __attribute__((packed));

#endif /* #ifndef _VESA_H */

// vim: ft=c ts=4 sts=4 sw=4 et ai cin
