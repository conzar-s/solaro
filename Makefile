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

$(BUILD)/version.inc: | $(BUILD)
	$(PYTHON) tools/mkversion.py $@

$(BUILD)/kernel.bin: kernel/kernel.asm kernel/console.inc kernel/disk.inc kernel/fat12.inc kernel/pe.inc $(BUILD)/version.inc | $(BUILD)
	$(ASM) -f bin -O0 -Wno-error=label-redef-late -I kernel/ -I $(BUILD)/ -o $@ $<

$(BUILD)/hello.bin: hello/hello.asm | $(BUILD)
	$(ASM) -f bin -o $@ $<

$(BUILD)/HELLO.EXE: $(BUILD)/hello.bin tools/mkpe.py | $(BUILD)
	$(PYTHON) tools/mkpe.py $(BUILD)/hello.bin $@

$(IMAGE): $(BUILD)/boot.bin $(BUILD)/kernel.bin $(BUILD)/HELLO.EXE tools/mkimage.py | $(BUILD)
	$(PYTHON) tools/mkimage.py $(BUILD)/boot.bin $(BUILD)/kernel.bin $@ $(BUILD)/HELLO.EXE

clean:
	$(PYTHON) -c "import shutil; shutil.rmtree('$(BUILD)', ignore_errors=True)"
