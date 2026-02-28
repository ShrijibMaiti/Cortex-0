; --- MULTIBOOT HEADER ---
MBALIGN  equ  1 << 0
MEMINFO  equ  1 << 1
FLAGS    equ  MBALIGN | MEMINFO
MAGIC    equ  0x1BADB002
CHECKSUM equ -(MAGIC + FLAGS)

section .multiboot
align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM

; --- STACK SETUP ---
section .bss
align 16
stack_bottom:
resb 16384 ; 16 KiB Stack
stack_top:

; --- KERNEL ENTRY POINT ---
section .text
global start
extern kmain

_start:
    mov esp, stack_top
    call kmain
    cli
.hang:
    hlt
    jmp .hang

; --- GDT LOAD (The Memory Fix) ---
global gdt_flush
extern gp
gdt_flush:
    lgdt [gp]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush2
.flush2:
    ret

; --- IDT LOAD (The Nervous System) ---
global idt_load
extern idtp
idt_load:
    lidt [idtp]
    ret

; --- KEYBOARD BRIDGE (The Crash Fix) ---
global keyboard_handler_asm
extern keyboard_handler

keyboard_handler_asm:
    pushad
    cld
    call keyboard_handler
    mov al, 0x20
    out 0x20, al
    popad
    iretd
