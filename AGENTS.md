# AGENTS.md

## Target

- CPU: Intel 8086 (16-bit real mode only).
- Boot medium: 360KB floppy disk image, raw sector layout.
- Floppy geometry: 40 cylinders, 2 heads, 9 sectors per track, 512 bytes per sector (720 sectors total).

## Project Structure

- `boot/boot.asm` - boot sector loaded by BIOS at `0x7C00`.
- `kernel/kernel.asm` - kernel loaded by the bootloader.
- `Makefile` - builds the bootable image and cleans artifacts.
- `build/` - generated binaries and disk image.

## Assembly Conventions

- All source is 16-bit x86 assembly for NASM (`bits 16`, `cpu 8086`).
- Use only instructions valid on the Intel 8086. Do not use 80186+ instructions (`pusha`/`popa`, shift by immediate count != 1, `movzx`, etc.).
- Keep source minimal and readable. Comment only non-obvious details.
- Keep bootloader and kernel load address/size assumptions consistent.

## Bootloader Responsibilities

- Occupy exactly one 512-byte boot sector, ending with `0xAA55`.
- Initialize segments and stack for a known real-mode environment.
- Load the kernel from floppy using BIOS INT 13h.
- Transfer execution to the kernel on success.
- Current layout: kernel is stored at disk sector 2 (CHS 0/0/2, LBA 1) and loaded to `0x0000:0x7E00`; `KERNEL_SECTORS` in `boot/boot.asm` controls the read count.

## Kernel Responsibilities

- Initialize the runtime environment it needs (segments, stack, direction flag as needed).
- Print `Hello, world!` using BIOS INT 10h teletype service.
- Halt the CPU in a low-power loop (`cli`/`hlt`/`jmp`).

## Build

- Requires `nasm`, `make`, and `python` on PATH.
- `make` builds `build/solaro.img`, a bootable 360KB floppy image.
- `make clean` removes `build/`.

## Technical Constraints

- The image must remain exactly 368640 bytes (360KB).
- Bootloader must stay within 512 bytes.
- Kernel binary size must not exceed `KERNEL_SECTORS * 512` bytes.
- BIOS loads only the first sector at boot; all additional code must be loaded explicitly by the bootloader.
- Runtime behavior is fixed: BIOS loads bootloader, bootloader loads kernel, kernel prints `Hello, world!`, CPU halts.
