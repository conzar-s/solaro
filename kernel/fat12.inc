; dl = drive number
list_drive:
    mov word [bpb_spt], 9
    mov word [bpb_heads], 2
    push dx
    mov ax, 0
    mov bx, SECTOR_BUF
    call read_lba
    pop dx
    jc .read_err

    cmp word [SECTOR_BUF + 510], 0xAA55
    jne .no_fs

    mov ax, [SECTOR_BUF + 11]   ; bytes per sector
    cmp ax, 512
    jne .no_fs

    mov ax, [SECTOR_BUF + 14]   ; reserved sectors
    mov [bpb_reserved], ax
    mov al, [SECTOR_BUF + 16]   ; FAT count
    xor ah, ah
    mov [bpb_fats], ax
    mov ax, [SECTOR_BUF + 22]   ; sectors per FAT
    mov [bpb_spf], ax
    mov ax, [SECTOR_BUF + 17]   ; root entries
    mov [bpb_root_entries], ax
    mov ax, [SECTOR_BUF + 24]   ; sectors per track
    mov [bpb_spt], ax
    mov ax, [SECTOR_BUF + 26]   ; heads
    mov [bpb_heads], ax

    ; root_dir_sectors = (root_entries * 32 + 511) / 512
    mov ax, [bpb_root_entries]
    mov cx, 32
    mul cx
    add ax, 511
    adc dx, 0
    mov cx, 512
    div cx
    mov [root_secs], ax
    cmp ax, 7
    jbe .root_secs_ok
    mov word [root_secs], 7
.root_secs_ok:

    ; root_lba = reserved + fats * spf
    mov ax, [bpb_fats]
    mul word [bpb_spf]
    add ax, [bpb_reserved]
    mov [root_lba], ax

    ; load FAT
    mov ax, [bpb_reserved]
    mov bx, FAT_BUF
    mov cx, [bpb_spf]
    cmp cx, 2
    jbe .fat_secs_ok
    mov cx, 2
.fat_secs_ok:
    mov dl, [drive]
.load_fat:
    push cx
    push ax
    push bx
    push dx
    call read_lba
    pop dx
    pop bx
    pop ax
    pop cx
    jc .read_err
    add ax, 1
    add bx, 512
    loop .load_fat

    ; load root directory
    mov ax, [root_lba]
    mov bx, ROOT_BUF
    mov cx, [root_secs]
    mov dl, [drive]
.load_root:
    push cx
    push ax
    push bx
    push dx
    call read_lba
    pop dx
    pop bx
    pop ax
    pop cx
    jc .read_err
    add ax, 1
    add bx, 512
    loop .load_root

    ; print entries
    mov ax, [root_secs]
    mov cx, 16
    mul cx
    cmp ax, [bpb_root_entries]
    jbe .max_entries
    mov ax, [bpb_root_entries]
.max_entries:
    mov cx, ax
    mov si, ROOT_BUF
.entry:
    push cx
    push si
    mov al, [si]
    cmp al, 0
    je .entries_done_pop
    cmp al, 0xE5
    je .next_entry
    mov al, [si + 11]
    test al, 0x08               ; volume label
    jnz .next_entry
    cmp al, 0x0F                ; LFN
    je .next_entry

    call print_entry
    mov si, msg_nl
    call print

.next_entry:
    pop si
    add si, 32
    pop cx
    loop .entry
    jmp .done

.entries_done_pop:
    pop si
    pop cx
.done:
    ret

.read_err:
    mov si, msg_read_err
    call print
    ret

.no_fs:
    mov si, msg_no_fs
    call print
    ret

; si = directory entry
print_entry:
    push si
    mov cx, 8
.name:
    lodsb
    cmp al, ' '
    je .name_done
    call putc
    loop .name
.name_done:
    pop si
    push si
    add si, 8
    cmp byte [si], ' '
    je .no_ext
    mov al, '.'
    call putc
    mov cx, 3
.ext:
    mov al, [si]
    cmp al, ' '
    je .ext_done
    call putc
    inc si
    loop .ext
.ext_done:
    pop si
    ret
.no_ext:
    pop si
    ret
