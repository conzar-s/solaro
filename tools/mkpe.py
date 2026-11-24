import struct
import sys

inp, outp = sys.argv[1], sys.argv[2]
code = open(inp, 'rb').read()

file_align = 0x200
sect_align = 0x1000
image_base = 0x00020000
rva_text = 0x1000
raw_text = 0x200
text_raw_size = (len(code) + file_align - 1) // file_align * file_align
text_virt_size = len(code)
size_of_headers = raw_text
size_of_image = (rva_text + ((text_virt_size + sect_align - 1) // sect_align * sect_align))

# DOS header
dos = bytearray(0x80)
dos[0:2] = b'MZ'
struct.pack_into('<H', dos, 0x3C, 0x80)

# PE signature
pe = bytearray(b'PE\x00\x00')

# COFF header
coff = bytearray(20)
struct.pack_into('<H', coff, 0, 0x014C)   # Machine i386
struct.pack_into('<H', coff, 2, 1)         # NumberOfSections
struct.pack_into('<I', coff, 4, 0)         # TimeDateStamp
struct.pack_into('<I', coff, 8, 0)         # PointerToSymbolTable
struct.pack_into('<I', coff, 12, 0)        # NumberOfSymbols
struct.pack_into('<H', coff, 16, 224)      # SizeOfOptionalHeader
struct.pack_into('<H', coff, 18, 0x0103)   # Characteristics (no relocs, executable, 32-bit)

# Optional header PE32
opt = bytearray(224)
struct.pack_into('<H', opt, 0, 0x10B)      # Magic PE32
opt[2] = 1  # MajorLinkerVersion
# SizeOfCode
struct.pack_into('<I', opt, 4, text_raw_size)
struct.pack_into('<I', opt, 16, rva_text)  # AddressOfEntryPoint
struct.pack_into('<I', opt, 20, rva_text)  # BaseOfCode
struct.pack_into('<I', opt, 24, rva_text)  # BaseOfData
struct.pack_into('<I', opt, 28, image_base)
struct.pack_into('<I', opt, 32, sect_align)
struct.pack_into('<I', opt, 36, file_align)
struct.pack_into('<H', opt, 40, 1)         # MajorOSVersion
struct.pack_into('<H', opt, 48, 1)         # MajorImageVersion
struct.pack_into('<H', opt, 52, 4)         # MajorSubsystemVersion
struct.pack_into('<I', opt, 56, size_of_image)
struct.pack_into('<I', opt, 60, size_of_headers)
struct.pack_into('<H', opt, 68, 3)         # Subsystem = console (not important)
struct.pack_into('<I', opt, 72, 0x100000)  # SizeOfStackReserve
struct.pack_into('<I', opt, 76, 0x1000)
struct.pack_into('<I', opt, 80, 0x100000)
struct.pack_into('<I', opt, 84, 0x1000)
struct.pack_into('<I', opt, 92, 16)        # NumberOfRvaAndSizes

# Section header .text
sec = bytearray(40)
sec[0:6] = b'.text\x00'
struct.pack_into('<I', sec, 8, text_virt_size)
struct.pack_into('<I', sec, 12, rva_text)
struct.pack_into('<I', sec, 16, text_raw_size)
struct.pack_into('<I', sec, 20, raw_text)
struct.pack_into('<I', sec, 36, 0x60000020)  # code, execute, read

headers = bytes(dos) + bytes(pe) + bytes(coff) + bytes(opt) + bytes(sec)
if len(headers) > raw_text:
    raise SystemExit('headers too large')

out = bytearray(raw_text + text_raw_size)
out[0:len(headers)] = headers
out[raw_text:raw_text + len(code)] = code
open(outp, 'wb').write(out)
