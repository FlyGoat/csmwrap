# detect architecture
MYARCH = $(shell uname -m | sed s,i[3456789]86,ia32,)
ifeq ($(ARCH),)
ARCH = $(MYARCH)
endif

# get source files, generate object names
ifeq ($(SRCS),)
SRCS = $(wildcard *.c) $(wildcard *.S) $(wildcard *.nasm)
endif
TMP = $(SRCS:.c=.o)
TMP1 = $(TMP:.S=.o)
OBJS = $(TMP1:.nasm=.o)
CFLAGS += -fshort-wchar -fno-strict-aliasing -ffreestanding -fno-stack-protector -fno-stack-check -I. -I./uefi \
  -I/usr/include -I/usr/include/efi -I/usr/include/efi/protocol -I/usr/include/efi/$(ARCH) -D__$(ARCH)__
ifeq ($(ARCH),x86_64)
CFLAGS += -DHAVE_USE_MS_ABI -mno-red-zone
endif

# for libuefi.
LIBSRCS = $(filter-out $(wildcard crt_*.c),$(wildcard *.c)) $(wildcard *.S)
TMP2 = $(LIBSRCS:.c=.o)
LIBOBJS = $(TMP2:.S=.o)

# NASM can only be nasm :-)
NASM = nasm
# detect toolchain
ifeq ($(wildcard /usr/bin/clang)$(wildcard /usr/local/bin/clang)$(wildcard /usr/lib/llvm/*/bin/clang),)
USE_GCC = 1
endif
ifneq ($(USE_GCC),)
ifeq ($(ARCH),x86_64)
CFLAGS += -maccumulate-outgoing-args
endif
NASMFLAGS += -f elf64
CFLAGS += -Wno-builtin-declaration-mismatch -fpic -fPIC
LDFLAGS += -nostdlib -shared -Bsymbolic -Luefi uefi/crt_$(ARCH).o
LIBS += -o $(TARGET).so -luefi -T uefi/elf_$(ARCH)_efi.lds
# see if we're cross-compiling
ifneq ($(ARCH),$(MYARCH))
CC = $(ARCH)-elf-gcc
LD = $(ARCH)-elf-ld
OBJCOPY ?= $(ARCH)-elf-objcopy
else
CC = gcc
LD = ld
OBJCOPY ?= objcopy
endif
ifeq ($(ARCH),aarch64)
EFIARCH = pei-aarch64-little
else
EFIARCH = efi-app-$(ARCH)
endif
AR ?= ar
else
# clang toolchian uses COFF64
NASMFLAGS += -f win64
CFLAGS += --target=$(ARCH)-pc-win32-coff -Wno-builtin-requires-header -Wno-incompatible-library-redeclaration -Wno-long-long
LDFLAGS += -subsystem:efi_application -nodefaultlib -dll -entry:uefi_init uefi/*.o
LIBS = -out:$(TARGET)
CC = clang
LD = lld -flavor link
OBJCOPY = true
endif

# recipies
ifeq ($(wildcard uefi/Makefile),)
ALLTARGETS = crt_$(ARCH).o libuefi.a build
else
ALLTARGETS = uefi/crt_$(ARCH).o uefi/libuefi.a $(OBJS) $(TARGET)
endif

all: $(ALLTARGETS) $(EXTRA)

uefi/libuefi.a:
	@make --no-print-directory -C uefi libuefi.a USE_GCC=$(USE_GCC) ARCH=$(ARCH)

libuefi.lib: $(LIBOBJS)

libuefi.a: $(LIBOBJS)
ifneq ($(USE_GCC),)
	@rm $@ 2>/dev/null || true
	$(AR) -frsv $@ $(LIBOBJS) >/dev/null
else
	@true
endif

$(TARGET): $(TARGET).so
ifneq ($(USE_GCC),)
	@$(OBJCOPY) -j .text -j .sdata -j .data -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target $(EFIARCH) --subsystem=10 $^ $@ || echo target: $(EFIARCH)
	@rm $(TARGET).so
endif

$(TARGET).so: $(OBJS)
	$(LD) $(LDFLAGS) $^ $(LIBS)
	@rm *.lib 2>/dev/null || true

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.nasm
	$(NASM) $(NASMFLAGS) $< -o $@

build:
	@mkdir ../build ../build/uefi 2>/dev/null || true
	@cp crt_$(ARCH).o ../build/uefi/crt0.o
	@cp elf_$(ARCH)_efi.lds ../build/uefi/link.ld
	@cp libuefi.a uefi.h ../build/uefi

clean:
	@rm $(TARGET) *.o *.a *.lib *.elf $(LIBOBJS) 2>/dev/null || true

distclean: clean
ifeq ($(wildcard uefi/Makefile),)
	@rm -rf ../build 2>/dev/null || true
else
	@rm uefi/*.o uefi/libuefi.a 2>/dev/null || true
endif
