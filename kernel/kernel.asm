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

    mov si, VERSION_STR
    call print
    mov si, msg_boot
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

    mov byte [drive], 0
.try_exec:
    mov dl, [drive]
    call mount_drive
    jc .next_exec
    mov si, hello_name
    call find_file
    jc .next_exec
    mov dl, [drive]
    call exec_pe
    mov si, msg_exit
    call print
    jmp .halt

.next_exec:
    inc byte [drive]
    cmp byte [drive], 2
    jb .try_exec
    mov si, msg_no_hello
    call print

.halt:
    cli
.hang:
    hlt
    jmp .hang

%include "kernel/console.inc"
%include "kernel/disk.inc"
%include "kernel/fat12.inc"
%include "kernel/pe.inc"

drive db 0
bpb_reserved dw 0
bpb_fats dw 0
bpb_spf dw 0
bpb_spc dw 0
bpb_root_entries dw 0
bpb_spt dw 9
bpb_heads dw 2
root_secs dw 0
root_lba dw 0
data_lba dw 0
lba_tmp dw 0
buf_tmp dw 0
drv_tmp db 0
chs_cx dw 0
chs_dh db 0
file_cluster dw 0
pe_drv db 0
pe_entry dw 0
pe_optsz dw 0
pe_nsects dw 0
pe_opt dw 0
pe_ep dw 0
pe_rawsz dw 0
pe_rawptr dw 0

msg_boot db "Booting kernel...", 13, 10, 0
msg_drive db "Drive ", 0
msg_colon db ":/", 13, 10, 0
msg_nl db 13, 10, 0
msg_read_err db "  Read error", 13, 10, 0
msg_no_fs db "  No FAT12 filesystem", 13, 10, 0
msg_exec db "Starting HELLO.EXE...", 13, 10, 0
msg_bad_mz db "  Bad MZ", 13, 10, 0
msg_bad_pe_sig db "  Bad PE sig", 13, 10, 0
msg_bad_opt db "  Bad PE opt", 13, 10, 0
msg_pe_load db "  File load error", 13, 10, 0
msg_no_hello db "  HELLO.EXE not found", 13, 10, 0
msg_exit db "Program exited.", 13, 10, 0
hello_name db "HELLO   EXE"

%include "version.inc"
