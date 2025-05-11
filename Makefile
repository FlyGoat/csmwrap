TARGET = csmwrap.efi
ARCH = x86_64
#USE_GCC=1

NASM ?= nasm

NASMSRCS=$(wildcard *.nasm)
EXTRA=$(NASMSRCS:.nasm=.o)

include uefi/Makefile

ifneq ($(USE_GCC),)
NASMFLAGS += -f elf64
else
NASMFLAGS += -f win64
endif

%.o: %.nasm
	$(NASM) $(NASMFLAGS) $< -o $@
