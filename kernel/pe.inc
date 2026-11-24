LOAD_SEG equ 0x0A40

; si = directory entry, dl = drive
; loads and executes PE/COFF image, returns on retf
exec_pe:
    push ax
    push bx
    push cx
    push dx
    push si
    push di
    push bp
    push es
    push ds

    mov [pe_drv], dl
    mov [pe_entry], si

    ; load file to LOAD_SEG:0000
    mov ax, LOAD_SEG
    mov es, ax
    xor bx, bx
    mov si, [pe_entry]
    mov dl, [pe_drv]
    call read_file
    jc .err

    ; validate DOS header
    cmp word es:[0], 0x5A4D         ; MZ
    jne .bad_mz

    mov bx, es:[0x3C]               ; e_lfanew
    cmp word es:[bx], 0x4550        ; PE
    jne .bad_pe
    cmp word es:[bx + 2], 0x0000
    jne .bad_pe

    ; COFF header at bx+4
    lea si, [bx + 4]
    mov ax, es:[si + 16]            ; SizeOfOptionalHeader
    mov [pe_optsz], ax
    mov ax, es:[si + 2]             ; NumberOfSections
    mov [pe_nsects], ax

    ; optional header at si+20
    add si, 20
    mov [pe_opt], si
    cmp word es:[si], 0x10B         ; PE32
    jne .bad_opt
    mov ax, es:[si + 16]            ; AddressOfEntryPoint
    mov [pe_ep], ax

    ; section headers
    mov si, [pe_opt]
    add si, [pe_optsz]
    mov cx, [pe_nsects]
.sect:
    cmp cx, 0
    je .run
    push cx
    push si

    mov ax, es:[si + 16]            ; SizeOfRawData
    mov [pe_rawsz], ax
    mov ax, es:[si + 20]            ; PointerToRawData
    mov [pe_rawptr], ax
    mov di, es:[si + 12]            ; VirtualAddress

    mov cx, [pe_rawsz]
    mov si, [pe_rawptr]
    push ds
    push es
    mov ax, LOAD_SEG
    mov ds, ax
    mov es, ax
    cld
    rep movsb
    pop es
    pop ds

    pop si
    add si, 40
    pop cx
    dec cx
    jmp .sect

.run:
    mov si, msg_exec
    call print

    mov ax, [pe_ep]
    mov [.farcall + 1], ax
    mov word [.farcall + 3], LOAD_SEG
    mov ax, LOAD_SEG
    mov ds, ax
    mov es, ax
.farcall:
    call 0x0A40:0000

    xor ax, ax
    mov ds, ax
    mov es, ax
    clc
    jmp .out

.bad_mz:
    mov si, msg_bad_mz
    call print
    stc
    jmp .out

.bad_pe:
    mov si, msg_bad_pe_sig
    call print
    stc
    jmp .out

.bad_opt:
    mov si, msg_bad_opt
    call print
    stc
    jmp .out

.err:
    mov si, msg_pe_load
    call print
    stc

.out:
    pop ds
    pop es
    pop bp
    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret
