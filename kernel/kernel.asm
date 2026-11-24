bits 16
cpu 8086
org 0x7E00

SECTOR_BUF equ 0x9000
FAT_BUF    equ 0x9200
ROOT_BUF   equ 0x9600

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti
    cld

    mov si, msg_hello
    call print

    mov byte [drive], 0
.drive_loop:
    mov si, msg_drive
    call print
    mov al, [drive]
    add al, 'A'
    call putc
    mov si, msg_colon
    call print

    mov dl, [drive]
    call list_drive

    inc byte [drive]
    cmp byte [drive], 2
    jb .drive_loop

    cli
.hang:
    hlt
    jmp .hang

%include "kernel/console.inc"
%include "kernel/disk.inc"
%include "kernel/fat12.inc"

drive db 0
bpb_reserved dw 0
bpb_fats dw 0
bpb_spf dw 0
bpb_root_entries dw 0
bpb_spt dw 9
bpb_heads dw 2
root_secs dw 0
root_lba dw 0
lba_tmp dw 0
buf_tmp dw 0
drv_tmp db 0
chs_cx dw 0
chs_dh db 0

msg_hello db "Hello, world!", 13, 10, 0
msg_drive db "Drive ", 0
msg_colon db ":/", 13, 10, 0
msg_nl db 13, 10, 0
msg_read_err db "  Read error", 13, 10, 0
msg_no_fs db "  No FAT12 filesystem", 13, 10, 0
