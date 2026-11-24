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

    mov al, [SECTOR_BUF + 13]   ; sectors per cluster
    xor ah, ah
    mov [bpb_spc], ax
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

    ; data_lba = root_lba + root_secs
    add ax, [root_secs]
    mov [data_lba], ax

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

; dl = drive number, loads BPB/FAT/root without printing
mount_drive:
    push ax
    push bx
    push cx
    push dx

    mov word [bpb_spt], 9
    mov word [bpb_heads], 2
    push dx
    mov ax, 0
    mov bx, SECTOR_BUF
    call read_lba
    pop dx
    jc .err

    cmp word [SECTOR_BUF + 510], 0xAA55
    jne .err
    mov ax, [SECTOR_BUF + 11]
    cmp ax, 512
    jne .err

    mov al, [SECTOR_BUF + 13]
    xor ah, ah
    mov [bpb_spc], ax
    mov ax, [SECTOR_BUF + 14]
    mov [bpb_reserved], ax
    mov al, [SECTOR_BUF + 16]
    xor ah, ah
    mov [bpb_fats], ax
    mov ax, [SECTOR_BUF + 22]
    mov [bpb_spf], ax
    mov ax, [SECTOR_BUF + 17]
    mov [bpb_root_entries], ax
    mov ax, [SECTOR_BUF + 24]
    mov [bpb_spt], ax
    mov ax, [SECTOR_BUF + 26]
    mov [bpb_heads], ax

    mov ax, [bpb_root_entries]
    mov cx, 32
    mul cx
    add ax, 511
    adc dx, 0
    mov cx, 512
    div cx
    mov [root_secs], ax
    cmp ax, 7
    jbe .rs_ok
    mov word [root_secs], 7
.rs_ok:
    mov ax, [bpb_fats]
    mul word [bpb_spf]
    add ax, [bpb_reserved]
    mov [root_lba], ax
    add ax, [root_secs]
    mov [data_lba], ax

    mov ax, [bpb_reserved]
    mov bx, FAT_BUF
    mov cx, [bpb_spf]
    cmp cx, 2
    jbe .fs_ok
    mov cx, 2
.fs_ok:
    mov dl, [drive]
.fat:
    push cx
    push ax
    push bx
    push dx
    call read_lba
    pop dx
    pop bx
    pop ax
    pop cx
    jc .err
    add ax, 1
    add bx, 512
    loop .fat

    mov ax, [root_lba]
    mov bx, ROOT_BUF
    mov cx, [root_secs]
    mov dl, [drive]
.root:
    push cx
    push ax
    push bx
    push dx
    call read_lba
    pop dx
    pop bx
    pop ax
    pop cx
    jc .err
    add ax, 1
    add bx, 512
    loop .root

    clc
    pop dx
    pop cx
    pop bx
    pop ax
    ret
.err:
    stc
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; si = 11-byte FAT name, returns si = entry ptr, CF clear if found
find_file:
    mov ax, [root_secs]
    mov cx, 16
    mul cx
    cmp ax, [bpb_root_entries]
    jbe .max_ok
    mov ax, [bpb_root_entries]
.max_ok:
    mov cx, ax
    mov di, ROOT_BUF
.next:
    push cx
    push si
    push di
    mov al, [di]
    cmp al, 0
    je .not_found_pop
    cmp al, 0xE5
    je .skip
    mov cx, 11
.cmp:
    mov al, [si]
    cmp al, [di]
    jne .skip
    inc si
    inc di
    loop .cmp
    pop si
    pop di
    pop cx
    clc
    ret
.skip:
    pop di
    pop si
    pop cx
    add di, 32
    loop .next
    stc
    ret
.not_found_pop:
    pop di
    pop si
    pop cx
    stc
    ret

; ax = cluster, returns ax = next cluster
fat12_next:
    push bx
    push cx
    push dx
    push si
    mov si, ax
    mov cx, ax
    shr cx, 1
    add ax, cx          ; ax = cluster * 3 / 2
    mov bx, FAT_BUF
    add bx, ax
    mov ax, [bx]
    test si, 1
    jz .even
    mov cl, 4
    shr ax, cl
    jmp .mask
.even:
    and ax, 0x0FFF
.mask:
    pop si
    pop dx
    pop cx
    pop bx
    ret

; ax = cluster, es:bx = buffer, dl = drive
; reads bpb_spc sectors, advances bx, CF on error
read_cluster:
    push ax
    push cx
    push dx

    sub ax, 2
    mov cx, [bpb_spc]
    mul cx
    add ax, [data_lba]
    mov cx, [bpb_spc]
.sec:
    push cx
    push ax
    push bx
    push dx
    call read_lba
    pop dx
    pop bx
    pop ax
    pop cx
    jc .err
    add ax, 1
    add bx, 512
    loop .sec
    clc
    jmp .out
.err:
    stc
.out:
    pop dx
    pop cx
    pop ax
    ret

; si = directory entry, es:bx = destination, dl = drive
; reads file data, returns CF on error
read_file:
    push ax
    push cx
    push dx
    push si
    push di

    mov ax, [si + 26]       ; first cluster
    mov [file_cluster], ax
.loop:
    mov ax, [file_cluster]
    cmp ax, 0x0FF8
    jae .done
    cmp ax, 2
    jb .done
    mov ax, [file_cluster]
    call read_cluster
    jc .err
    mov ax, [file_cluster]
    call fat12_next
    mov [file_cluster], ax
    jmp .loop
.done:
    clc
    jmp .out
.err:
    stc
.out:
    pop di
    pop si
    pop dx
    pop cx
    pop ax
    ret
