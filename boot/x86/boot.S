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

        .section        .bootstrap_stack, "aw", "nobits"
        .align          16
stack_bottom:
        .skip           16384
stack_top:

        .section        .bss, "aw", "nobits"
        .align 4
IDT:
    .Length:
        .word           0
    .Base:
        .long           0

        .equ            CPUID_EXTENSIONS, 0x80000000
        .equ            CPUID_FEATURES, 0x80000001

        .equ            CR0_PAGING, 1 << 31
        .equ            PT_SIZE, 4096
        .equ            PML4T_ADDR, 0x1000
        .equ            PDPT_ADDR, 0x2000
        .equ            PDT_ADDR, 0x3000
        .equ            PT_ADDR, 0x4000

        .equ            PT_ADDR_MASK, 0xFFFFFFFFFF000
        .equ            PT_PRESENT, 1
        .equ            PT_READABLE, 2

        .equ            PT_ENTRIES, 512
        .equ            PT_ENTRY_SIZE, 8
        .equ            PAGE_SIZE, 0x1000

        .equ            CR4_PAE_ENABLE, 1 << 5

        .equ            EFER_MSR, 0xC0000080
        .equ            EFER_LM_ENABLE, 1 << 8

        .equ            CR0_PM_ENABLE, 1 << 0
        .equ            CR0_WB_ENABLE, 1 << 5
        .equ            CR0_PG_ENABLE, 1 << 31

        .section        .multiboot.text, "aw"
        .code32
        .global         _start
        .type           _start,     @function
_start:
        mov             CPUID_EXTENSIONS, %eax
        cpuid
        cmp             CPUID_FEATURES, %eax
        jb              1f
        jmp             2f
1:
        mov             %al, 0x0E
        mov             %bh, 0
        mov             %bl, 7
        mov             %cx, 1

        mov             %al, debug_real
        int             0x10
        jmp             1b

2:
        /* Enable A20 line */
        mov             %al, 0x92
        mov             %al, 2
        mov             0x92, %al


        /* Disable 32bit paging */
        movl            %cr0, %eax
        andl            ~CR0_PAGING, %eax
        movl            %eax, %cr0

        /* clear the tables */
        movl            PML4T_ADDR, %edi
        movl            %cr3, %edi /* Set CR3 telling the CPU where the tables are */
        xorl            %eax, %eax
        movl            %ecx, PT_SIZE
        rep             stosl /* Write 4*PT_SIZE bytes for the 4 tables */
        movl            %edi, %cr3

        /* Link up first entries of each table */
        movl            %edi, PML4T_ADDR
        movl            %edx, PDPT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE
        movl            (%edi), %edx
        movl            %edi, PDPT_ADDR
        movl            %edx, PDT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE
        movl            (%edi), %edx
        movl            %edi, PDT_ADDR
        movl            %edx, PT_ADDR & PT_ADDR_MASK | PT_PRESENT | PT_READABLE
        movl            (%edi), %edx

        /* Fill page table */
        movl            %edi, PT_ADDR
        movl            $0, %esi
        movl            %ebx, PT_PRESENT | PT_READABLE
        movl            %ecx, PT_ENTRIES
3:
        cmpl            0x1000, %esi
        jl              4f
        cmpl            0x200000, %esi
        jge             5f

        movl            %esi, %edx
        orl             %ebx, %edx
        movl            %edx, (%edi)

4:
        addl            PAGE_SIZE, %esi
        addl            PT_ENTRY_SIZE, %edi
        loop            1b

5:
        /* Identity map kernel 0-384 0x0 -> 0x3FFFFF 0xC0000000 -> 0xC03FFFFF */
        movl            $(PT_ADDR | PT_PRESENT | PT_READABLE), PDT_ADDR - 0xC0000000
        movl            $(PT_ADDR | PT_PRESENT | PT_READABLE), PDT_ADDR - 0xC0000000 + 384 * PT_ENTRY_SIZE

        movl            $(PDT_ADDR - 0xC0000000), %eax
        movl            %eax, %cr3

        /* Disable IRQs */
        mov             %al, 0xFF
        mov             0xA1, %al
        mov             0x21, %al

        nop
        nop

        /* Load zero length IDT so any NMI causes triple fault */
        lidt            IDT

        /* Enable PAE */
        movl            %eax, %cr4
        orl             %eax, CR4_PAE_ENABLE
        movl            %cr4, %eax

        /* Switch to compatability mode */
        movl            %ecx, EFER_MSR
        rdmsr
        orl             %eax, EFER_LM_ENABLE
        wrmsr

        /* Enable paging and protected mode */
        movl            %eax, %cr0
        orl             %eax, CR0_PG_ENABLE | CR0_PM_ENABLE | CR0_WB_ENABLE
        movl            %cr0, %eax

        /* Debug */
        movl            %esi, debug_protected
        call            print_protected

        .equ            VGA_MEMORY, 0xC03FF000
        .equ            VGA_HEIGHT, 80
        .equ            VGA_WIDTH, 25
        .equ            VGA_BPL, 2
        .equ            VGA_BUFFER_SIZE, (VGA_HEIGHT * VGA_WIDTH * VGA_BPL)
        /* Jump to real mode */
        lgdt            Pointer
fjt:
        .quad           real
        .word           8
        jmp             *fjt

        .section        .text
        .code64
real:
        cli

        mov             %ax, Data - 0xC0000000
        mov             %ds, %ax
        mov             %es, %ax
        mov             %fs, %ax
        mov             %gs, %ax
        mov             %ss, %ax

        /* movq            (VGA_MEMORY), %rdi */
        /* movq            %rcx, 500 */
        /* movq            %rax, 0x1F201F201F201F20 */
        /* rep             stosq */

        /* movq            (VGA_MEMORY), %rdi */
        /* movq            %rax, 0x1F6C1F6C1F651F48 */
        /* movq            (%rdi), %rax */

        call            kernel_entry


        cli
.halt:
        hlt
        jmp             .halt

        /* 16-bit real mode print function */
        .section        .multiboot.text, "aw"
        .code16
        .global         print_real
        .type           print_real, @function
print_real:
        /* Input: %esi = null-terminated string */
        /* Clobbers: %eax, %edi */
        pushl           %ebp
        movl            %esp, %ebp
        pushl           %edi
        movl            0xB800, %eax
        movl            %eax, %es
.Lprint_real_loop:
        lodsb
        cmpb            %al, 0
        je              .Lprint_real_newline
        movb            %ah, 0x07
        movl            %es:(%edi), %eax
        addl            %edi, 2
        jmp             .Lprint_real_loop
.Lprint_real_newline:
        addl            160, %edi
        popl            %edi
        popl            %ebp
        ret
debug_real:
        .asciz          "[boot] Real mode"

        /* 32-bit protected mode print function */
        .code32
        .global         print_protected
        .type           print_protected, @function
print_protected:
        /* Input: %esi = null-terminated string */
        /* Clobbers: %eax, %edi */
        pushl           %ebp
        movl            %ebp, %esp
        pushl           %edi
        movl            %edi, VGA_MEMORY
        addl            %edi, %eax
.Lprint_protected_loop:
        lodsb
        cmpb            %al, 0
        je              .Lprint_protected_newline
        movb            %ah, 0x07
        movl            (%edi), %eax
        addl            %edi, 2
        jmp             .Lprint_protected_loop
.Lprint_protected_newline:
        addl            %edi, 160
        subl            %edi, VGA_MEMORY
        popl            %edi
        popl            %ebp
        ret
debug_protected:
        .asciz          "[boot] Protected mode"

        .equ            PRESENT, 1 << 7
        .equ            NOT_SYS, 1 << 4
        .equ            EXEC, 1 << 3
        .equ            DC, 1 << 2
        .equ            RW, 1 << 1
        .equ            ACCESSED, 1 << 0

        .equ            GRAN_4K, 1 << 7
        .equ            SZ_32, 1 << 6
        .equ            LONG_MODE, 1 << 5

        .section        .data, "aw"
        .align          8
        .globl          GDT
GDT:
    Null:
        .quad 0
    Code:
        .Code_limit_lo:
            .word       0xFFFF
        .Code_base_lo:
            .word       0
        .Code_base_mid:
            .byte       0
        .Code_access:
            .byte       PRESENT | NOT_SYS | EXEC | RW
        .Code_flags:
            .byte       GRAN_4K | LONG_MODE | 0xF  # Flags & imit (high, bits 16-19)
        .Code_base_hi:
            .byte       0
    Data:
        .Data_limit_lo:
            .word       0xFFFF
        .Data_base_lo:
            .word       0
        .Data_base_mid:
            .byte       0
        .Data_access:
            .byte       PRESENT | NOT_SYS | RW
        .Data_Flags:
            .byte       GRAN_4K | SZ_32 | 0xF     # Flags & imit (high, bits 16-19)
        .Data_base_hi:
            .byte       0
    Pointer:
        .word           . - GDT - 1
        .quad           GDT

/* vim: ft=asm ts=4 sts=4 sw=4 et ai cin */
