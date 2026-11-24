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

$(BUILD)/kernel.bin: kernel/kernel.asm | $(BUILD)
	$(ASM) -f bin -o $@ $<

$(IMAGE): $(BUILD)/boot.bin $(BUILD)/kernel.bin | $(BUILD)
	$(PYTHON) -c "import sys; b=open(sys.argv[1],'rb').read(); k=open(sys.argv[2],'rb').read(); img=bytearray(368640); img[:len(b)]=b; img[512:512+len(k)]=k; open(sys.argv[3],'wb').write(img)" $(BUILD)/boot.bin $(BUILD)/kernel.bin $@

clean:
	$(PYTHON) -c "import shutil; shutil.rmtree('$(BUILD)', ignore_errors=True)"
