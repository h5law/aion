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
else
	$(error "Unsupported architecture: $(ARCH)")
endif

DIAG = -Wall -Wextra -Wpedantic
OFLAGS = -O2

CFLAGS ?= $(OFLAGS) $(DIAG) -g
CPPFLAGS ?=
LDFLAGS ?=
INCS ?=

INCS := -I$(AION_PATH)/lib/include -I$(AION_PATH)/include
CFLAGS := $(CFLAGS) -nostdinc -ffreestanding -ffunction-sections -fdata-sections $(INCS)
LDFLAGS := $(LDFLAGS) -nostdlib

SRCDIRS = stdio stdlib string

LIBK_SRCS = $(foreach dir,$(SRCDIRS),$(wildcard libk/$(dir)/*.c))
LIBK_OBJS = $(LIBK_SRCS:.c=.libk.o)

LIBS = libk.a

.PHONY: all build clean

all: $(LIBS)

clean:
	rm -f $(LIBS)
	rm -f $(LIBK_OBJS) *.o */*.o */*/*.o
	rm -f $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d

%.libk.o: %.S
	$(CC) -MD $(CFLAGS) -o $@ -c $<

%.libk.o: %.c
	$(CC) -MD $(CFLAGS) -o $@ -c $<

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $^

build: $(LIBS)
