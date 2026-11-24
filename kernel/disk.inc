; ax = LBA, bx = buffer, dl = drive
; returns CF on error
read_lba:
    push ax
    push bx
    push cx
    push dx
    push si
    push di

    mov [lba_tmp], ax
    mov [buf_tmp], bx
    mov [drv_tmp], dl

    mov ax, [bpb_spt]
    cmp ax, 0
    jne .spt_ok
    mov word [bpb_spt], 9
.spt_ok:
    mov ax, [bpb_heads]
    cmp ax, 0
    jne .heads_ok
    mov word [bpb_heads], 2
.heads_ok:
    ; sector = (LBA % SPT) + 1
    mov ax, [lba_tmp]
    xor dx, dx
    div word [bpb_spt]
    mov cl, dl
    inc cl
    ; ax = LBA / SPT, head = ax % heads, cyl = ax / heads
    xor dx, dx
    div word [bpb_heads]
    mov dh, dl
    mov ch, al

    mov [chs_cx], cx
    mov [chs_dh], dh
    mov di, 3
.retry:
    mov ah, 0x00
    mov dl, [drv_tmp]
    int 0x13
    mov cx, [chs_cx]
    mov dh, [chs_dh]
    mov dl, [drv_tmp]
    mov bx, [buf_tmp]
    mov ah, 0x02
    mov al, 1
    int 0x13
    jnc .ok
    dec di
    jnz .retry
    stc
    jmp .out
.ok:
    clc
.out:
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret
