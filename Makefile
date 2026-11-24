ASM = nasm
PYTHON = python
BUILD = build
IMAGE = $(BUILD)/solaro.img

.PHONY: all clean

all: $(IMAGE)

$(BUILD):
	$(PYTHON) -c "import os; os.makedirs('$(BUILD)', exist_ok=True)"

$(BUILD)/boot.bin: boot/boot.asm | $(BUILD)
	$(ASM) -f bin -o $@ $<

$(BUILD)/kernel.bin: kernel/kernel.asm kernel/console.inc kernel/disk.inc kernel/fat12.inc | $(BUILD)
	$(ASM) -f bin -I kernel/ -o $@ $<

$(IMAGE): $(BUILD)/boot.bin $(BUILD)/kernel.bin tools/mkimage.py | $(BUILD)
	$(PYTHON) tools/mkimage.py $(BUILD)/boot.bin $(BUILD)/kernel.bin $@

clean:
	$(PYTHON) -c "import shutil; shutil.rmtree('$(BUILD)', ignore_errors=True)"
