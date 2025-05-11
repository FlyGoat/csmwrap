TARGET = csmwrap.efi
ARCH = x86_64
#USE_GCC=1

NASM ?= nasm

NASMSRCS=$(wildcard *.nasm)
EXTRA=$(NASMSRCS:.nasm=.o)

%.o: %.nasm
	$(NASM) $< -o $@

include uefi/Makefile
