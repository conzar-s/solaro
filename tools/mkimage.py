import sys

boot_path, kernel_path, out_path = sys.argv[1], sys.argv[2], sys.argv[3]

SECTOR = 512
TOTAL = 720
RESERVED = 9
FATS = 2
SPF = 2
ROOT_ENTRIES = 112

boot = open(boot_path, "rb").read()
kernel = open(kernel_path, "rb").read()

if len(boot) != SECTOR:
    raise SystemExit("boot sector must be 512 bytes")
if len(kernel) > (RESERVED - 1) * SECTOR:
    raise SystemExit("kernel too large for reserved sectors")

img = bytearray(TOTAL * SECTOR)
img[0:SECTOR] = boot
img[SECTOR:SECTOR + len(kernel)] = kernel

fat_offset = RESERVED * SECTOR
fat = bytearray(SPF * SECTOR)
fat[0:3] = bytes([0xFD, 0xFF, 0xFF])
for i in range(FATS):
    off = fat_offset + i * SPF * SECTOR
    img[off:off + len(fat)] = fat

open(out_path, "wb").write(img)
