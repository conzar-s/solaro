bits 16
org 0x1000

start:
    push cs
    pop ds
    mov si, msg
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0x00
    int 0x10
    jmp .loop
.done:
    retf

msg db "Hello from HELLO.EXE!", 13, 10, 0
