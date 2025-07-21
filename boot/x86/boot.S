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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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

        .section        .multiboot.data, "aw"
        .align          8
multiboot_header:
        .long           0xE85250D6
        .long           0
        .long           multiboot_header_end - multiboot_header
        .long           -(0xE85250D6 + 0 + (multiboot_header_end - multiboot_header))
        .word           0
        .word           0
        .long           8
multiboot_header_end:

        .section        .bootstrap_stack, "aw", @nobits
        .align          16
stack_bottom:
        .skip           16384
stack_top:

        .section        .bss, "aw", @nobits
        .align          4096
boot_page_directory:
        .skip 4096
boot_page_table1:
        .skip 4096

        .section        .rodata
eflags_msg:
        .asciz          "[boot] clearing eflags registers\r\n"
boot_msg:
        .asciz          "[boot] multiboot2 kernel starting...\r\n"
not_multiboot_msg:
        .asciz          "[error] Not loaded by Multiboot2 bootloader!\r\n"
state_ok_msg:
        .asciz          "[boot] Machine state verification passed\r\n"
state_error_msg:
        .asciz          "[error] Machine state verification failed!\r\n"
kernel_msg:
        .asciz          "[kernel] Kernel entrypoint called\r\n"
halt_msg:
        .asciz          "[boot] Kernel returned, halting system\r\n"

        .section        .multiboot.text, "aw"
        .global         _start
        .type           _start,     @function
_start:
        /* Physical address of boot_page_table1 */
        movl            $(boot_page_table1 - 0xC0000000), %edi
        /* Map 0 address */
        movl            $0, %esi
        movl            $1023, %ecx

1:
        /* Map the kernel */
        cmpl            $_kernel_start, %esi
        jl              2f
        cmpl            $(_kernel_end - 0xC0000000), %esi
        jge             3f

        /* Map physical address as present and writable */
        movl            %esi, %edx
        orl             $0x003, %edx
        movl            %edx, (%edi)

2:
        /* Map boot pages */
        addl            $4096, %esi
        addl            $4, %edi
        loop            1b

3:
        /* Map VGA VM to 0xC03FF000 as present, writable */
        movl            $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

        /* Identity map the page table to both virtual addresses
         * 0x00000000 and 0xC0000000 */
	    movl            $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
	    movl            $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

        /* Set CR3 to the boot page directory */
        movl            $(boot_page_directory - 0xC0000000), %ecx
        movl            %ecx, %cr3

        /* Enable paging and write protect bit */
        movl            %cr0, %ecx
        orl             $0x80010000, %ecx
        movl            %ecx, %cr0

        /* Jump to higher half kernel (absolute) */
        lea             4f, %ecx
        jmp             *%ecx

        .section        .text
4:
        /* Unmap identity boot page directory mapping */
        movl            $0, boot_page_directory + 0

        /* Force TBL flush */
        movl            %cr3, %ecx
        movl            %ecx, %cr3

        movl            $stack_top,                 %esp

        call            kernel_entry

        pushl           $halt_msg
        call            printf
        addl            $4,                         %esp
        cli
.halt:
        hlt
        jmp             1b

/* vim: ft=asm ts=4 sts=4 sw=4 et ai cin */
