AS 		 = as

BUILD 	 = build
BOOT_DIR = boot

BOOT_SOURCES = $(wildcard $(BOOT_DIR)/*.S) $(wildcard $(BOOT_DIR)/*.s)

.PHONY: clean bootloader

always:
	mkdir -pv $(BUILD)
	mkdir -pv $(BOOT_DIR)

clean: always
	rm -vfrd $(BUILD)

bootloader: always $(BOOT_SOURCES)
	$(AS) -o $(BUILD)/boot $(BOOT_SOURCES)
