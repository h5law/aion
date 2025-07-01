/* boot.S
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

/* Multiboot header definition */
    .set        ALIGN,     1<<0                /* align modules loaded to page boundaries */
    .set        MEMINFO,   1<<1                /* provide a memory map */
    .set        FLAGS,     ALIGN | MEMINFO     /* set multiboot flag field options */
    .set        MAGIC,     0x1BADB002          /* multiboot header magic id number */
    .set        CHECKSUM,  -(MAGIC + FLAGS)    /* checksum to validate the header */

/* Multiboot Header section */
    .section    .multiboot
    .align      4
.long   MAGIC
.long   FLAGS
.long   CHECKSUM

/* bss data section
 * Multiboot does **not** provide an initial stack pointer (esp)
 * Allocate a dedicated small section for kernel stack space */
    .section    .bss
    .align      16      /* x86 requires 16-bit aligned stack */
stack_bottom:
    .skip       16384   /* Skip over the 16KiB stack */
stack_top:

/* text section
 * Define the kernel entry point (_start)
 * Initialise things needed prior to kernel entry
 * Call into the kernel's entry point looping forever */
    .section    .text
    .global     _start
    .type       _start,     @function
_start:
    /* Initialise some form of debugging (printf) */
    /* Setup hardware, load GDT, define Interrupts, enable Paging */
    mov     $stack_top,     %esp        /* Set the stack pointer to our stack */
    call    kernel_entry                /* Call the kernel_entry function */
    cli                                 /* Disable (clear) interrupts */
1:  hlt                                 /* Exited (halt) from kernel_entry */
    jmp     1b                          /* Jump back to halt interrupt instruction */
.size       _start,     . - _start

/* vim: ft=asm ts=4 sts=4 sw=4 et ai cin */
