/* kernel.c
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

#include <stdio.h>
#include <stdlib.h>

#include <kernel/x86/multiboot2.h>
#include <kernel/psf.h>
#include <kernel/vga.h>
#include <kernel/vesa.h>

void kernel_entry(uint32_t magic, uint32_t addr);
void kernel_entry(uint32_t magic, uint32_t addr)
{
    vga_init();
    vga_setcolour(VGA_COLOUR_BLACK, VGA_COLOUR_WHITE);
    printf("                                    aionOS                    "
           "                \n");
    vga_setcolour(VGA_COLOUR_WHITE, VGA_COLOUR_BLACK);

    struct multiboot_tag *tag;
    size_t                size;

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        printf("[multiboot2] Invalid magic number: 0x%x\n", ( unsigned )magic);
        abort();
    }

    if (addr & 7) {
        printf("[multiboot2] Unaligned mbi: 0x%x\n", addr);
        abort();
    }

    size = ( uintptr_t )addr;
    printf("[multiboot2] Announced mbi size 0x%x\n", ( unsigned int )size);
    for (tag = ( struct multiboot_tag * )( uintptr_t )(addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = ( struct multiboot_tag * )(( multiboot_uint8_t * )tag +
                                          ((tag->size + 7) & ~7))) {
        printf("[multiboot2] Tag 0x%x\n"
               "             Size 0x%x\n",
               tag->type, tag->size);
        switch (tag->type) {
        case MULTIBOOT_TAG_TYPE_CMDLINE:
            printf("[multiboot2] Command line = %s\n",
                   (( struct multiboot_tag_string * )tag)->string);
            break;
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
            printf("[multiboot2] Boot loader name = %s\n",
                   (( struct multiboot_tag_string * )tag)->string);
            break;
        case MULTIBOOT_TAG_TYPE_MODULE:
            printf("[multiboot2] Module at 0x%x-0x%x\n"
                   "             Tag command line %s\n",
                   (( struct multiboot_tag_module * )tag)->mod_start,
                   (( struct multiboot_tag_module * )tag)->mod_end,
                   (( struct multiboot_tag_module * )tag)->cmdline);
            break;
        case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
            printf("[multiboot2] Tag memory info\n"
                   "                   mem_lower = %uKB\n"
                   "                   mem_upper = %uKB\n",
                   (( struct multiboot_tag_basic_meminfo * )tag)->mem_lower,
                   (( struct multiboot_tag_basic_meminfo * )tag)->mem_upper);
            break;
        case MULTIBOOT_TAG_TYPE_BOOTDEV:
            printf("[multiboot2] Boot device 0x%x, %u, %u\n",
                   (( struct multiboot_tag_bootdev * )tag)->biosdev,
                   (( struct multiboot_tag_bootdev * )tag)->slice,
                   (( struct multiboot_tag_bootdev * )tag)->part);
            break;
        case MULTIBOOT_TAG_TYPE_MMAP: {
            multiboot_memory_map_t *mmap;

            printf("[multiboot2] memory mapped devices\n");

            for (mmap = (( struct multiboot_tag_mmap * )tag)->entries;
                 ( multiboot_uint8_t * )mmap <
                 ( multiboot_uint8_t * )tag + tag->size;
                 mmap = ( multiboot_memory_map_t
                                  * )(( unsigned long )mmap +
                                      (( struct multiboot_tag_mmap * )tag)
                                              ->entry_size))
                printf("[multiboot2]     - base_addr = 0x%x\n"
                       "                   length = 0x%x\n"
                       "                   type = 0x%x\n",
                       ( unsigned )(mmap->addr >> 32) |
                               ( unsigned )(mmap->addr & 0xFFFFFFFF),
                       ( unsigned )(mmap->len >> 32) |
                               ( unsigned )(mmap->len & 0xFFFFFFFf),
                       ( unsigned )mmap->type);
        } break;
        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
            multiboot_uint32_t                color;
            unsigned                          i;
            struct multiboot_tag_framebuffer *tagfb =
                    ( struct multiboot_tag_framebuffer * )tag;
            void *fb =
                    ( void * )( unsigned long )tagfb->common.framebuffer_addr;

            printf("[vbe] VESA VBE Framebuffer address: 0x%x\n",
                   ( unsigned int )( uintptr_t )fb);

            printf("[vbe] Framebuffer specification: width = %u\n"
                   "                                 height = %u\n"
                   "                                 bpp = %u\n",
                   tagfb->common.framebuffer_width,
                   tagfb->common.framebuffer_height,
                   tagfb->common.framebuffer_bpp);

            // int len = pfs_puts(tagfb, "Chat are we cooked?\n");
            // printf("[vbe] Wrote %d characters to the framebuffer\n", len);

            switch (tagfb->common.framebuffer_type) {
            case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED: {
                unsigned                best_distance, distance;
                struct multiboot_color *palette;

                palette       = tagfb->framebuffer_palette;

                color         = 0;
                best_distance = 4 * 256 * 256;

                for (i = 0; i < tagfb->framebuffer_palette_num_colors; i++) {
                    distance = (0xff - palette[i].blue) *
                                       (0xff - palette[i].blue) +
                               palette[i].red * palette[i].red +
                               palette[i].green * palette[i].green;
                    if (distance < best_distance) {
                        color         = i;
                        best_distance = distance;
                    }
                }
            } break;

            case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
                color = ((1 << tagfb->framebuffer_blue_mask_size) - 1)
                        << tagfb->framebuffer_blue_field_position;
                break;

            case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
                color = '\\' | 0x0100;
                break;

            default:
                color = 0xffffffff;
                break;
            }

            for (i = 0; i < tagfb->common.framebuffer_width &&
                        i < tagfb->common.framebuffer_height;
                 i++) {
                switch (tagfb->common.framebuffer_bpp) {
                case 8: {
                    multiboot_uint8_t *pixel =
                            fb + tagfb->common.framebuffer_pitch * i + i;
                    *pixel = color;
                } break;
                case 15:
                case 16: {
                    multiboot_uint16_t *pixel =
                            fb + tagfb->common.framebuffer_pitch * i + 2 * i;
                    *pixel = color;
                } break;
                case 24: {
                    multiboot_uint32_t *pixel =
                            fb + tagfb->common.framebuffer_pitch * i + 3 * i;
                    *pixel = (color & 0xffffff) | (*pixel & 0xff000000);
                } break;

                case 32: {
                    multiboot_uint32_t *pixel =
                            fb + tagfb->common.framebuffer_pitch * i + 4 * i;
                    *pixel = color;
                } break;
                }
            }
            break;
        }
        }
    }

    tag = ( struct multiboot_tag * )(( multiboot_uint8_t * )tag +
                                     ((tag->size + 7) & ~7));

    printf("[multiboot2] Total mbi size 0x%x\n",
           ( int )(( uintptr_t )tag - addr));
}

// vim: ft=c ts=4 sts=4 sw=4 et ai cin
