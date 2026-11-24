import os
import struct
import sys

boot_path, kernel_path, out_path = sys.argv[1], sys.argv[2], sys.argv[3]
extra_files = sys.argv[4:]

SECTOR = 512
TOTAL = 720
RESERVED = 9
FATS = 2
SPF = 2
ROOT_ENTRIES = 112
ROOT_SECS = (ROOT_ENTRIES * 32 + SECTOR - 1) // SECTOR
SPC = 2

boot = open(boot_path, "rb").read()
kernel = open(kernel_path, "rb").read()

if len(boot) != SECTOR:
    raise SystemExit("boot sector must be 512 bytes")
if len(kernel) > (RESERVED - 1) * SECTOR:
    raise SystemExit("kernel too large for reserved sectors")

img = bytearray(TOTAL * SECTOR)
img[0:SECTOR] = boot
img[SECTOR:SECTOR + len(kernel)] = kernel

fat = bytearray(SPF * SECTOR)
fat[0:3] = bytes([0xFD, 0xFF, 0xFF])

data_start = RESERVED + FATS * SPF + ROOT_SECS
next_cluster = 2

def set_fat12(cluster, value):
    off = cluster * 3 // 2
    if cluster & 1:
        fat[off] = (fat[off] & 0x0F) | ((value & 0x0F) << 4)
        fat[off + 1] = (value >> 4) & 0xFF
    else:
        fat[off] = value & 0xFF
        fat[off + 1] = (fat[off + 1] & 0xF0) | ((value >> 8) & 0x0F)

root = bytearray(ROOT_SECS * SECTOR)
root_idx = 0

for path in extra_files:
    data = open(path, "rb").read()
    name = os.path.basename(path).upper()
    if "." in name:
        base, ext = name.rsplit(".", 1)
    else:
        base, ext = name, ""
    base = base[:8].ljust(8)
    ext = ext[:3].ljust(3)

    clusters = (len(data) + SPC * SECTOR - 1) // (SPC * SECTOR)
    if clusters == 0:
        clusters = 0
        first = 0
    else:
        first = next_cluster
        prev = None
        for i in range(clusters):
            c = next_cluster
            next_cluster += 1
            if prev is not None:
                set_fat12(prev, c)
            prev = c
            start = (data_start + (c - 2) * SPC) * SECTOR
            chunk = data[i * SPC * SECTOR:(i + 1) * SPC * SECTOR]
            img[start:start + len(chunk)] = chunk
        set_fat12(prev, 0xFFF)

    e = bytearray(32)
    e[0:8] = base.encode("ascii")
    e[8:11] = ext.encode("ascii")
    e[11] = 0x20
    struct.pack_into("<H", e, 26, first)
    struct.pack_into("<I", e, 28, len(data))
    root[root_idx:root_idx + 32] = e
    root_idx += 32

fat_offset = RESERVED * SECTOR
for i in range(FATS):
    off = fat_offset + i * SPF * SECTOR
    img[off:off + len(fat)] = fat

root_offset = (RESERVED + FATS * SPF) * SECTOR
img[root_offset:root_offset + len(root)] = root

open(out_path, "wb").write(img)
