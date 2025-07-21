/* linker.ld
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

ENTRY(_start)

PHDRS
{
    mdata   PT_LOAD FLAGS(6); /* Read + Write   (RW) */
    mtext   PT_LOAD FLAGS(5); /* Read + Write   (RW) */
    text    PT_LOAD FLAGS(5); /* Read + Execute (RX) */
    rodata  PT_LOAD FLAGS(4); /* Read-only      (RO) */
    data    PT_LOAD FLAGS(6); /* Read + Write   (RW) */
    bss     PT_LOAD FLAGS(6); /* Read + Write   (RW) */
}

/* The kernel will live at 3GB + 1MB in the virtual address space, */
/* which will be mapped to 1MB in the physical address space. */
SECTIONS
{
    . = 0x00100000;

    _kernel_start = .;
    .multiboot.data :
    {
        KEEP(*(.multiboot.data))
    } :mdata

    multiboot.text :
    {
       KEEP(*(.multiboot.text))
    } :mtext

    . += 0xC0000000;
	.text ALIGN (4K) : AT (ADDR (.text) - 0xC0000000)
	{
        . = ALIGN(4);
        KEEP(*(.init))
        . = ALIGN(4);
        __preinit_array_start = .;
        KEEP (*(.preinit_array))
        __preinit_array_end = .;

        . = ALIGN(4);
        __init_array_start = .;
        KEEP (*(SORT(.init_array.*)))
        KEEP (*(.init_array))
        __init_array_end = .;

        . = ALIGN(4);
        KEEP(*(.fini))

        . = ALIGN(4);
        __fini_array_start = .;
        KEEP (*(.fini_array))
        KEEP (*(SORT(.fini_array.*)))
        __fini_array_end = .;

        . = ALIGN(4);
        KEEP (*crtbegin.o(.ctors))
        KEEP (*(EXCLUDE_FILE (*crtend.o) .ctors))
        KEEP (*(SORT(.ctors.*)))
        KEEP (*crtend.o(.ctors))

        KEEP(*(.text .text.*))
	} :text

	.rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xC0000000)
	{
		KEEP(*(.rodata .rodata.*))
	} :rodata

	.data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000)
	{
		KEEP(*(.data .data.*))
	} :data

	.bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000)
	{
		KEEP(*(COMMON))
		KEEP(*(.bss .bss.*))
		KEEP(*(.bootstrap_stack))
	} :bss
	_kernel_end = .;
}

/* vim: ft=ld ts=4 sts=4 sw=4 et ai cin */
