# AGENTS.md

## Target

- CPU: Intel 8086 (16-bit real mode only).
- Boot medium: 360KB floppy disk image, raw sector layout with FAT12 BPB.
- Floppy geometry: 40 cylinders, 2 heads, 9 sectors per track, 512 bytes per sector (720 sectors total).

## Project Structure

- `boot/boot.asm` - boot sector with FAT12 BPB, loaded by BIOS at `0x7C00`.
- `kernel/kernel.asm` - kernel entry point; initializes runtime, prints boot messages, lists FAT12 root directories on floppy drives 0 and 1, and runs `HELLO.EXE`.
- `kernel/console.inc` - BIOS teletype output routines.
- `kernel/disk.inc` - LBA-to-CHS disk reads via BIOS INT 13h.
- `kernel/fat12.inc` - FAT12 BPB parsing, root directory listing, file lookup, and cluster-chain reads.
- `kernel/pe.inc` - minimal PE/COFF loader and executor for 16-bit real-mode programs.
- `hello/hello.asm` - sample 16-bit program built as `HELLO.EXE`.
- `tools/mkimage.py` - builds a bootable 360KB FAT12 image from boot, kernel, and optional files.
- `tools/mkpe.py` - wraps a flat 16-bit binary in a minimal PE32/COFF executable.
- `tools/mkversion.py` - generates `build/version.inc` with the current git commit hash.
- `Makefile` - builds the bootable image and cleans artifacts.
- `build/` - generated binaries, version include, and disk image.

## Assembly Conventions

- All source is 16-bit x86 assembly for NASM (`bits 16`, `cpu 8086`).
- Use only instructions valid on the Intel 8086. Do not use 80186+ instructions (`pusha`/`popa`, shift by immediate count != 1, `movzx`, etc.).
- Keep source minimal and readable. Comment only non-obvious details.
- Kernel modules are NASM include files (`.inc`) included by `kernel/kernel.asm`; keep shared buffer addresses, labels, and data definitions consistent.
- Keep bootloader, kernel, image layout, and load address/size assumptions consistent.

## Bootloader Responsibilities

- Occupy exactly one 512-byte boot sector, ending with `0xAA55`.
- Include a valid FAT12 BPB for a 360KB floppy.
- Initialize segments and stack for a known real-mode environment.
- Load the kernel from reserved sectors using BIOS INT 13h.
- Transfer execution to the kernel on success.
- Current layout: `ReservedSectors = KERNEL_SECTORS + 1`, kernel starts at disk sector 2 (CHS 0/0/2, LBA 1), loaded to `0x0000:0x7E00`; `KERNEL_SECTORS` in `boot/boot.asm` controls the read count.

## Kernel Responsibilities

- Initialize the runtime environment it needs (segments, stack, direction flag).
- Print the version banner and boot messages using BIOS INT 10h teletype service.
- Read FAT12 root directory contents from floppy drive 0 and drive 1 using BIOS INT 13h and BPB/FAT/root metadata.
- Load and execute `HELLO.EXE` from a FAT12 floppy using the minimal PE/COFF loader.
- Report read, filesystem, or loader errors and halt the CPU in a low-power loop.

## Build

- Requires `nasm`, `make`, `python`, and `git` on PATH.
- `make` generates `build/version.inc`, builds boot/kernel/HELLO.EXE, and produces `build/solaro.img`, a bootable 360KB FAT12 floppy image.
- `make clean` removes `build/`.

## Technical Constraints

- The image must remain exactly 368640 bytes (360KB).
- Bootloader must stay within 512 bytes.
- Kernel binary size must not exceed `KERNEL_SECTORS * 512` bytes.
- BIOS loads only the first sector at boot; all additional code must be loaded explicitly by the bootloader.
- The filesystem driver targets FAT12 floppies with 512-byte sectors; buffer sizes in the kernel assume standard 360KB FAT/root sizes unless expanded.
- PE/COFF loading is minimal: PE32 with MZ header, sections copied to `LOAD_SEG` (0x2000) by RVA, entry point far-called; executables must be 16-bit real-mode binaries linked for that layout.
