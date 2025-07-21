MAKEFLAGS += --no-builtin-rules --no-builtin-variables

ifdef TARGETARCH
	ARCHDIR = $(TARGETARCH)
else
	ARCHDIR = $(shell uname -m)
endif
ifeq ($(ARCHDIR),arm64)
	ARCHDIR = aarch64
else ifeq ($(ARCHDIR),amd64)
	ARCHDIR = x86_64
else ifeq ($(ARCHDIR),i386)
	ARCHDIR = x86
else ifeq ($(ARCHDIR),i686)
	ARCHDIR = x86
else ifeq ($(ARCHDIR),x86_64)
	ARCHDIR = x86
else
	$(error "Unsupported architecture: $(ARCH)")
endif

DIAG = -Wall -Wextra -Wpedantic
OFLAGS = -O2

CFLAGS ?= $(OFLAGS) $(DIAG) -g
CPPFLAGS ?=
LDFLAGS ?=
LIBS ?=
INCS ?=

INCS := -I$(AION_PATH)/include
LIBS := -L$(AION_PATH)/lib -lk
CFLAGS := $(CFLAGS) -nostdinc -ffreestanding $(INCS)
LDFLAGS := $(LDFLAGS) -nostdlib $(LIBS)

BOOTDIR = boot
KERNDIR = kern sys sys/$(ARCHDIR)

include $(BOOTDIR)/make.config

KERN_SRCS = $(foreach dir,$(KERNDIR),$(wildcard $(dir)/*.c))
KERN_OBJS = $(KERN_SRCS:.c=.o)

OBJS = \
			$(BOOTDIR)/$(ARCHDIR)/boot.o \
			$(KERN_OBJS) \

OS = aion
TARGET = $(OS)-$(ARCHDIR).kernel

.PHONY: all build clean grub

all: build grub

clean:
	rm -frd $(TARGET) $(OS).iso iso/
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

%.o: %.S
	$(CC) -MD $(CFLAGS) -o $@ -c $<

%.o: %.c
	$(CC) -MD $(CFLAGS) -o $@ -c $<

$(BOOTDIR)/$(ARCHDIR)/crtbegin.o $(BOOTDIR)/$(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

$(TARGET): $(OBJS)
	$(CC) -T linker.ld $(CFLAGS) -o $(TARGET) $(OBJS) $(LDFLAGS)
	grub-file --is-x86-multiboot2 $(TARGET)

build: $(TARGET)

grub: $(TARGET)
	mkdir -p iso/boot/grub/
	cp $(TARGET) iso/boot
	cp grub.cfg iso/boot/grub/grub.cfg
	grub-mkrescue -o $(OS).iso iso

qemu: grub
	qemu-system-i386                                     \
		  -accel tcg,thread=single                       \
		  -cpu core2duo                                  \
		  -m 128                                         \
		  -drive format=raw,media=cdrom,file=aion.iso    \
		  -serial stdio                                  \
		  -smp 1                                         \
		  -usb                                           \
		  -vga std
