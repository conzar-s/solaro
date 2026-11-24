; al = char
putc:
    push ax
    push bx
    mov ah, 0x0E
    mov bh, 0x00
    int 0x10
    pop bx
    pop ax

    push ax
    push dx
    push cx
    mov cx, 0xFFFF
    mov dx, 0x3FD
.wait:
    in al, dx
    test al, 0x20
    jnz .send
    loop .wait
    pop cx
    pop dx
    pop ax
    ret
.send:
    pop cx
    pop dx
    pop ax
    push dx
    mov dx, 0x3F8
    out dx, al
    pop dx
    ret

; si = zero-terminated string
print:
    push ax
    push si
.loop:
    lodsb
    test al, al
    jz .done
    call putc
    jmp .loop
.done:
    pop si
    pop ax
    ret
