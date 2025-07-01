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

/*****************************************************************************/
/*                               Return Status                               */
/*****************************************************************************/

/* AL Return Status Values */
#define VBE_STATUS_FUNC_SUPPORTED 0x4F /* Function is supported */

/* AH Return Status Values */
#define VBE_STATUS_SUCCESS        0x00 /* Function call successful */
#define VBE_STATUS_FAILED         0x01 /* Function call failed */
#define VBE_STATUS_NOT_SUPPORTED  0x02 /* Call not supported by config */
#define VBE_STATUS_INVALID        0x03 /* Call invalid in current mode */
/* ... */                              /* Any non-zero code is failure */

/*****************************************************************************/
/*                            VBE Data Structs                               */
/*****************************************************************************/

/* 14 bit wide mode number with flags and arguments detailing a VBE mode
 *      - [15] Preserve Display Memory Select
 *           - 0 Clear display memory
 *           - 1 Preserve display memory
 *      - [14] Linear/Flat Frame Buffer Select
 *           - 0 Use Banked/Windowed Frame Buffer
 *           - 1 Use Linear/Flat Frame Buffer
 *      - [13-12] - Reserved for VBE/AF
 *                **must** be 0
 *      - [11] Refresh Rate Control Select
 *           - 0 Use current BIOS rate
 *           - 1 Use CRTC value specified
 *      - [10-9] Reserved by VESA for future
 *             **must** be 0
 *      - [8] Mode Status Bit
 *          - 0 not a VESA defined VBE mode
 *          - 1 this is a VESA VBE Mode
 *      - [7-0] Mode Number */
typedef uint16_t vbe_mode_num_t;

/* DWORD pointer for the VBE referencing modes
 *      - Real mode (points below 1MB system memoru boundary)
 *           - 0xFFFF0000 Segment Mask
 *           - 0x0000FFFF Offset Mask
 *      - Protected mode is a 16 bit pointer
 *           - 0x0000FF00 Selector Mask
 *           - 0x000000FF Offset Mask */
typedef uint32_t vbe_far_ptr_t;

/* Definition of the different VESA VBE memory model types enumerated */
typedef enum {
    mem_text     = 0x00, /* Text mode */
    mem_cga      = 0x01, /* CGA graphics */
    mem_hercules = 0x02, /* Hercules graphics */
    mem_planar   = 0x03, /* Planar memory model */
    mem_packed   = 0x04, /* Packed pixel memory model */
    mem_nchain   = 0x05, /* Non-chain 4, 256 color */
    mem_drgb     = 0x06, /* Direct color RGB memory model */
    mem_dyuv     = 0x07, /* Direct color YUV memory model */
    /* 0x08 - 0x0F Reserved for VESA future usage */
    /* 0x10 - 0xFF Defined by the OEM */
} mem_model_e;

/* Starting with VBE/Core 3.0, all the VBE functions are optionally accessible
 * from 16-bit and 32- bit protected mode applications and operating systems via
 * a new ‘Protected Mode Entry Point’. The protected mode entry point defines a
 * special location that can be used to directly call the VBE functions as
 * 16-bit protected mode code. The application or OS does not call the BIOS code
 * directly from protected mode, but first makes a copy of the BIOS image in a
 * writeable section of memory and then calls the code within this relocated
 * memory block. The entry point is located within a special ‘Protected Mode
 * Information Block’, which will be located somewhere within the first 32Kb of
 * the BIOS image (if this information block is not found, then the BIOS does
 * not support this new interface) */
struct pm_info_block_t {
    char     signature[4];   /* "PMID" - PM Info Block Signature */
    uint16_t entry_point;    /* Offset of PM entry point within BIOS */
    uint16_t pm_initialize;  /* Offset of PM initialization entry point */
    uint16_t bios_data_sel;  /* Selector to BIOS data area emulation block
                              * (initialise to 0) */

    uint16_t a0000_sel;      /* Selector to access A0000h physical mem
                              * (initialise to 0xA000) */
    uint16_t b0000_sel;      /* Selector to access B0000h physical mem
                              * (initialise to 0xB000) */
    uint16_t b8000_sel;      /* Selector to access B8000h physical mem
                              * (initialise to 0xB800) */
    uint16_t code_seg_sel;   /* Selector to access code segment as data
                              * (initialise to 0xC000) */

    uint8_t in_protect_mode; /* Set to 1 when in protected mode (init: 0) */
    uint8_t checksum;        /* Checksum byte for structure */
} __attribute__((packed));

/* VBE General structure for getting and receiving VBE generic information */
struct vbe_info_block_t {
    char          vbe_signature[4]; /* "@ESA" - VBE Signature */
    uint16_t      vbe_version;      /* VBE Version (init: 0x0300) */
    vbe_far_ptr_t oem_string_ptr;   /* Pointer to OEM String */

    /* Capabilities of graphics controller
     *      `- [31-5] Reserved flag bits
     *       - [4] Stereo signalling bit
     *          - 0 supported via external VESA stereo connector
     *          - 1 supported via VESA EVC connector
     *       - [3] Stereoscopic signalling bit
     *          - 0 no stereoscopic hardware support
     *          - 1 hardware stereoscopic support by a controller
     *       - [2] RAMDAC bits
     *          - 0 Normal RAMDAC operation
     *          - 1 Use a blank bit in function 0x09 for large RAMDAC usage
     *       - [1] VGA Compatability bit
     *          - 0 VGA compatible
     *          - 1 Not VGA compatible
     *       - [0] DAC width flag
     *          - 0 Fixed 6-bit witdth per primary colour
     *          - 1 Switchable 6 to bits per primary colour */
    uint32_t capabilities;

    vbe_far_ptr_t video_mode_ptr; /* Pointer to VideoModeList ends in 0xFFFF
                                   * The list consists of 16 bit mode numbers */

    uint16_t total_memory;        /* Number of 64KB memory blocks */

    /* Added for VBE 2.0+ */
    uint16_t      oem_software_rev; /* VBE implementation Software revision */
    vbe_far_ptr_t oem_vendor_name_ptr;  /* Pointer to Vendor Name String */
    vbe_far_ptr_t oem_product_name_ptr; /* Pointer to Product Name String */
    vbe_far_ptr_t oem_product_rev_ptr;  /* Pointer to Product Revision String */

    uint8_t reserved[222]; /* Reserved for VBE implementation scratch area */
    uint8_t oem_data[256]; /* Data Area for OEM Strings */
} __attribute__((packed));

/* VBE Mode Information structure for getting and receiving mode info */
struct mode_info_block_t {
    /* Mandatory information for all VBE revisions */
    /* Mode attribute bit mask values
     *      - [15-13] Reserved
     *      - [12] Dual display start address support
     *            - 0 = No
     *            - 1 = Yes
     *      - [11] Hardware stereoscopic display support
     *            - 0 = No
     *            - 1 = Yes
     *      - [10] Hardware triple buffering support
     *            - 0 = No
     *            - 1 = Yes
     *      - [9] Interlaced mode is available
     *            - 0 = No
     *            - 1 = Yes
     *      - [8] Double scan mode is available
     *            - 0 = No
     *            - 1 = Yes
     *      - [7] Linear frame buffer mode is available
     *            - 0 = No
     *            - 1 = Yes
     *      - [6] VGA compatible windowed memory mode is available
     *            - 0 = Yes
     *            - 1 = No
     *      - [5] VGA compatible mode
     *            - 0 = Yes
     *            - 1 = No
     *      - [4] Mode type
     *            - 0 = Text mode
     *            - 1 = Graphics mode
     *      - [3] = Monochrome/color mode (see note below)
     *            - 0 = Monochrome mode
     *            - 1 = Color mode
     *      - [2] TTY Output functions supported by BIOS
     *            - 0 = TTY Output functions not supported by BIOS
     *            - 1 = TTY Output functions supported by BIOS
     *      - [1] Reserved (set to 1)
     *      - [0] Mode supported by hardware configuration
     *             - 0 = Mode not supported in hardware
     *             - 1 = Mode supported in hardware */
    uint16_t mode_attributes;

    /* Window A attributes
     *      - [7-3] Reserved
     *      - [2] Window writeable
     *            - 0 = Window is not writeable
     *            - 1 = Window is writeable
     *      - [1] Window readable
     *            - 0 = Window is not readable
     *            - 1 = Window is readable
     *      - [0] Relocatable window(s) supported
     *            - 0 = Single non-relocatable window only
     *            - 1 = Relocatable window(s) are supported */
    uint8_t win_a_attributes;

    /* Window B attributes
     *      - [7-3] Reserved
     *      - [2] Window writeable
     *            - 0 = Window is not writeable
     *            - 1 = Window is writeable
     *      - [1] Window readable
     *            - 0 = Window is not readable
     *            - 1 = Window is readable
     *      - [0] Relocatable window(s) supported
     *            - 0 = Single non-relocatable window only
     *            - 1 = Relocatable window(s) are supported */
    uint8_t win_b_attributes;

    uint16_t win_granularity;     /* granularity in KB (smallest boundry) */
    uint16_t win_size;            /* window size */
    uint16_t win_a_segment;       /* window A start segment */
    uint16_t win_b_segment;       /* window B start segment */
    uint32_t win_func_ptr;        /* real mode pointer to window function */
    uint16_t bytes_per_scan_line; /* bytes per scan line */

    /* Mandatory information for VBE 1.2 and above */
    uint16_t x_resolution; /* horizontal resolution in pixels or characters */
    uint16_t y_resolution; /* vertical resolution in pixels or characters */
    uint8_t  x_char_size;  /* character cell width in pixels */
    uint8_t  y_char_size;  /* character cell height in pixels */
    uint8_t  number_of_planes;         /* number of memory planes */
    uint8_t  bits_per_pixel;           /* bits per pixel */
    uint8_t  number_of_banks;          /* number of banks */
    mem_model_e memory_model;          /* memory model type */
    uint8_t     bank_size;             /* bank size in KB */
    uint8_t     number_of_image_pages; /* number of images */

    uint8_t reserved1;                 /* reserved for page function */

    /* Direct Color fields (required for direct/6 and YUV/7 memory models) */
    uint8_t red_mask_size;   /* size of direct color red mask in bits */
    uint8_t red_field_pos;   /* bit position of lsb of red mask */
    uint8_t green_mask_size; /* size of direct color green mask in bits */
    uint8_t green_field_pos; /* bit position of lsb of green mask */
    uint8_t blue_mask_size;  /* size of direct color blue mask in bits */
    uint8_t blue_field_pos;  /* bit position of lsb of blue mask */
    uint8_t rsvd_mask_size;  /* size of direct color reserved mask in bits */
    uint8_t rsvd_field_pos;  /* bit position of lsb of reserved mask */

    /* Direct colour mode attributes
     *      - [1] Usability of bits in the rsvd field
     *          - 0 bits are reserved
     *          - 1 bits are usable by the application
     *      - [0] State of colour ramp
     *          - 0 ramp is fixed
     *          - 1 ramp is programmable */
    uint8_t direct_color_mode_info;

    /* Mandatory information for VBE 2.0 and above */
    uint32_t phys_base_ptr; /* physical address for flat memory frame buffer */

    uint32_t reserved2;     /* Reserved - always set to 0 */
    uint16_t reserved3;     /* Reserved - always set to 0 */

    /* Mandatory information for VBE 3.0 and above */
    uint16_t lin_bytes_per_scan_line; /* bytes per scan line for linear modes */
    uint8_t  bnk_number_of_image_pages; /* number of images for banked modes */
    uint8_t  lin_number_of_image_pages; /* number of images for linear modes */
    uint8_t  lin_red_mask_size; /* direct color red mask size in linear modes */
    uint8_t  lin_red_field_pos; /* LSB pos of red mask linear modes */
    uint8_t lin_green_mask_size; /* direct color green mask size linear modes */
    uint8_t lin_green_field_pos; /* bit pos of lsb of green mask linear modes */
    uint8_t lin_blue_mask_size;  /* direct color blue mask size linear modes */
    uint8_t lin_blue_field_pos;  /* LSB position of blue mask linear modes */
    uint8_t lin_rsvd_mask_size;  /* direct color rsvd mask size linear modes */
    uint8_t lin_rsvd_field_pos; /* LSB position of reserved mask linear modes */
    uint32_t max_pixel_clock;   /* max pixel clock (in Hz) for graphics mode */

    uint8_t reserved4[189];     /* remainder of mode_info_block_t */
} __attribute__((packed));

struct crtc_info_block_t {
    uint16_t horizontal_total;      /* Horizontal total in pixels */
    uint16_t horizontal_sync_start; /* Horizontal sync start in pixels */
    uint16_t horizontal_sync_end;   /* Horizontal sync end in pixels */
    uint16_t vertical_total;        /* Vertical total in lines */
    uint16_t vertical_sync_start;   /* Vertical sync start in lines */
    uint16_t vertical_sync_end;     /* Vertical sync end in lines */

    /* CRTC Flag attributes
     *      - [3] Vertical sync polarity
     *          - 0 = Vertical sync polarity is positive (+)
     *          - 1 = Vertical sync polarity is negative (-)
     *     - [2] Horizontal sync polarity
     *          - 0 = Horizontal sync polarity is positive (+)
     *          - 1 = Horizontal sync polarity is negative (-)
     *     - [1] Interlaced Mode Enable
     *          - 0 = Graphics mode is non-interlaced
     *          - 1 = Graphics mode is interlaced
     *     - [0] Double Scan Mode Enable
     *          - 0 = Graphics mode is not double scanned
     *          - 1 = Graphics mode is double scanned */
    uint8_t flags;         /* Flags (Interlaced, Double Scan etc) */

    uint32_t pixel_clock;  /* Pixel clock in units of Hz */
    uint16_t refresh_rate; /* Refresh rate in units of 0.01 Hz */
    uint8_t  reserved[40]; /* remainder of CRTCInfoBlock */
} __attribute__((packed));

struct vbe2_pmi_table {
    uint16_t set_window;        /* table offset for prot. mode fn code 0x4F05 */
    uint16_t set_display_start; /* table offset for prot. mode fn code 0x4F07 */
    uint16_t set_pallette;      /* table offset for prot. mode fn code 0x4F09 */
    /* List of Ports terminating with 0xFFFF then Memory Location base addresses
     * 4 bytes each, the Limit of Length-1 in 2 bytes terminating with 0xFFFF
     * Port/Index combo 0x03DE/0x03DF and Memory locations DE800-DEA00h (length
     * = 200h)
     *      - DE 03 DF 03 FF FF 00 E8 0D 00 00 01 FF FF */
    uint16_t ports_mem_ptr; /* Optional offsets of ports and memory locations */
    /* ... Variable remainder of table including code */
} __attribute__((packed));

struct pallette_entry_t {
    uint8_t blue;      /* Blue channel value (6 or 8 bits) */
    uint8_t green;     /* Green channel value (6 or 8 bits) */
    uint8_t red;       /* Red channel value (6 or 8 bits) */
    uint8_t alignment; /* DWORD alignment byte (unused) */
} __attribute__((packed));

/*****************************************************************************/
/*                              Function Calls                               */
/*****************************************************************************/

/* Get VBE information function
 * Input:   AX    = 0x4F00 - function code
 *          ES:DI = Segment:Offset to where vbe_info_t can be found
 * Output:  AX    = 0x4F00 (success) indicates ES:DI has valid information
 *                = 0xXXXX (failure) vbe information request failed bios error
 *                                   ES:DI may have invalid or corrupted data */
#define VESA_FUNC_GET_VBE_INFO            0x4F00

/* Get VBE mode information function
 * Input:   AX    = 0x4F01 - function code
 *          CX    = VESA mode number from video modes array
 *          ES:DI = Segment:Offset pointer to where vbe_mode_info_t can be found
 * Output:  AX    = 0x4F00 (success) indicates ES:DI has valid information
 *                = 0xXXXX (failure) mode (CV) is unsupported or bios error
 *                                   ES:DI may have invalid or corrupted data */
#define VESA_FUNC_GET_VBE_MODE_INFO       0x4F01

/* Set VBE mode function
 * Input:   AX    = 0x4F02 - function code
 *          BX    = vbe_mode_num_t
 * Output:  AX    = 0x004F (success) the VBE mode has been set successfully
 *                = 0xXXXX (failure) indicates errors such as:
 *                                        - BIOS errors
 *                                        - Too little video memory
 *                                        - Unsupported VBE mode
 *                                        - Mode doesnt support LFB
 *                                        - ... */
#define VESA_FUNC_SET_VBE_MODE            0x4F02

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
#define VESA_FUNC_GET_VBE_MODE            0x4F03

/* Save/Restore state function
 * Input:  AX   = 0x4F04 - function mode
 *         DL   = 00h Return Save/Restore State buffer size
 *                 - 01h Save state
 *                 - 02h Restore state
 *         CX   = Requested states
 *                 [0] = Save/Restore controller hardware state
 *                 [1] = Save/Restore BIOS data state
 *                 [2] = Save/Restore DAC state
 *                 [3] = Save/Restore Register state
 *         ES:BX = Pointer to buffer (if DL == 00h)
 * Output: AX   = VBE Return Status
 *         BX   = Number of 64-byte blocks to hold the state
 *                buffer (if DL=00h) */
#define VESA_FUNC_SAVE_RESTORE_STATE      0x4F04

/* Display Window Control function
 * Input:   AX    = 0x4F05 - function code
 *          BH    = 0x00 - Set bank/memory window (Force on 32 bit input)
 *                  0x01 - Get bank/memory window
 *          BL    = 0x00 - Window A
 *                  0x01 - Window B
 *          DX    = 0xXXXX - Bank/Window number in window granularity units
 *          ES    = Selector for memory mapped registers (32 bit input only)
 * Output:  AX    = 0x004F (success) indicates the operation was a success
 *                = 0xXXXX (failure) indicates errors such as:
 *                                       - BIOS errors
 *                                       - Unusable bank
 *                                       - Unpresent bank
 *                                       - Not in bank switching mode
 *                                       - ...
 *         DX     = Bank/Window number in window granularity units (get only) */
#define VESA_FUNC_VBE_DWC                 0x4F05

/* Set/Get Logical Scan Line Length function
 * Input:   AX    = 0x4F06 - function code
 *          BL    = 0x00 - Set Scan Line Length in Pixels
 *                  0x01 - Get Scan Line Length
 *                  0x02 - Set Scan Line Length in Bytes
 *                  0x03 - Get Maximum Scan Line Length
 *          CX    = If BL=00h Desired Width in Pixels
 *                  If BL=02h Desired Width in Bytes
 *                  (Ignored for Get Functions)
 * Output:  AX    = VBE Return Status
 *          BX    = Bytes Per Scan Line
 *          CX    = Actual Pixels Per Scan Line (truncated to nearest complete
 * pixel) DX    = Maximum Number of Scan Lines */
#define VESA_FUNC_SET_GET_SCAN_LINE       0x4F06

/* Set/Get Display Start function
 * Input:   AX    = 0x4F07 - function code
 *          BH    = 0x00 - Reserved and must be 00h (16-bit mode)
 *          BL    = 0x00 - Set Display Start
 *                  0x01 - Get Display Start
 *                  0x02 - Schedule Display Start (Alternate)
 *                  0x03 - Schedule Stereoscopic Display Start
 *                  0x04 - Get Scheduled Display Start Status
 *                  0x05 - Enable Stereoscopic Mode
 *                  0x06 - Disable Stereoscopic Mode
 *                  0x80 - Set Display Start during Vertical Retrace
 *                  0x82 - Set Display Start during Vertical Retrace (Alternate)
 *                  0x83 - Set Stereoscope Display Start during Vertical Retrace
 *          ECX   = If BL=0x02/0x82 Display Start Address in bytes
 *                  If BL=0x03/0x83 Left Image Start Address in bytes
 *          EDX   = If BL=0x03/0x83 Right Image Start Address in bytes
 *          CX    = If BL=0x00/0x80 First Displayed Pixel In Scan Line
 *          DX    = If BL=0x00/0x80 First Displayed Scan Line
 *          ES    = Selector for memory mapped registers (32-bit mode)
 * Output:  AX    = VBE Return Status
 *          BH    = If BL=0x01 Reserved and will be 0
 *          CX    = If BL=0x01 First Displayed Pixel In Scan Line
 *                  If BL=0x04 0 if flip has not occurred, not 0 if it has
 *          DX    = If BL=0x01 First Displayed Scan Line */
#define VESA_FUNC_SET_GET_0xDISPLAY_START 0x4F07

/* Set/Get DAC Palette Format function
 * Input:   AX    = 0x4F08 - function code
 *          BL    = 0x00 - Set DAC Palette Format
 *                  0x01 - Get DAC Palette Format
 *          BH    = Desired bits of colour per primary (Set only)
 * Output:  AX    = VBE Return Status
 *          BH    = Current number of bits of colour per primary */
#define VESA_FUNC_SET_GET_DAC_PALETTE     0x4F08

/* Set/Get Palette Data function
 * Input:   AX    = 0x4F09 - function code
 *          BL    = 0x00 - Set Palette Data
 *                  0x01 - Get Palette Data
 *                  0x02 - Set Secondary Palette Data
 *                  0x03 - Get Secondary Palette Data
 *                  0x80 - Set Palette Data during Vertical Retrace
 *          CX    = Number of palette registers to update (to a maximum of 256)
 *          DX    = First of the palette registers to update (start)
 *          ES:DI = Table of palette values (16-bit mode)
 *          ES:EDI= Table of palette values (32-bit mode)
 *          DS    = Selector for memory mapped registers (32-bit mode)
 * Output:  AX    = VBE Return Status */
#define VESA_FUNC_SET_GET_PALETTE_DATA    0x4F09

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
#define VESA_FUNC_GET_VBE_PMI             0x4F0A

/* Get/Set pixel clock function
 * Input:   AX    = 0x4F0B - function code
 *          BL    = 0x00 - Get closest pixel clock
 *          ECX   = Requested pixel clock in units of Hz
 *          DX    = Mode number pixel clock will be used with
 * Output:  AX    = VBE Return Status
 *          ECX   = Closest pixel clock (BL = 00h) */
#define VESA_FUNC_GET_SET_PIXEL_CLOCK     0x4F0B

#endif /* #ifndef _VESA_H */

// vim: ft=c ts=4 sts=4 sw=4 et ai cin
