bits 16
cpu 8086
org 0x7C00

KERNEL_OFFSET equ 0x7E00
KERNEL_SECTORS equ 8

start:
    jmp short boot
    nop

    db "SOLARO  "
    dw 512
    db 2
    dw KERNEL_SECTORS + 1
    db 2
    dw 112
    dw 720
    db 0xFD
    dw 2
    dw 9
    dw 2
    dd 0
    dd 0
    db 0
    db 0
    db 0x29
    dd 0x12345678
    db "SOLARO     "
    db "FAT12   "

boot:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov [boot_drive], dl

    mov si, 3
.read:
    mov ah, 0x00
    mov dl, [boot_drive]
    int 0x13

    mov ah, 0x02
    mov al, KERNEL_SECTORS
    mov ch, 0x00
    mov cl, 0x02
    mov dh, 0x00
    mov dl, [boot_drive]
    mov bx, KERNEL_OFFSET
    int 0x13
    jnc .loaded

    dec si
    jnz .read

    mov si, disk_error
    call print
    jmp halt

.loaded:
    jmp 0x0000:KERNEL_OFFSET

print:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0x00
    int 0x10
    jmp print
.done:
    ret

halt:
    cli
.hang:
    hlt
    jmp .hang

boot_drive db 0
disk_error db "Disk read error", 0

times 510 - ($ - $$) db 0
dw 0xAA55
