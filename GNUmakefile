# Nuke built-in rules and variables.
MAKEFLAGS += -rR
.SUFFIXES:

# This is the name that our final executable will have.
# Change as needed.
override OUTPUT := csmwrap

# Target architecture to build for. Default to x86_64.
ARCH := x86_64

# Check if the architecture is supported.
ifeq ($(filter $(ARCH),ia32 x86_64),)
    $(error Architecture $(ARCH) not supported)
endif

# Default user QEMU flags. These are appended to the QEMU command calls.
QEMUFLAGS := -m 2G

# User controllable C compiler command.
CC := cc

# User controllable objcopy command.
OBJCOPY := objcopy

# User controllable C flags.
CFLAGS := -g -O2 -pipe

# User controllable C preprocessor flags. We set none by default.
CPPFLAGS :=

# User controllable nasm flags.
NASMFLAGS := -F dwarf -g

# User controllable linker flags. We set none by default.
LDFLAGS :=

# Check if CC is Clang.
override CC_IS_CLANG := $(shell ! $(CC) --version 2>/dev/null | grep 'clang' >/dev/null 2>&1; echo $$?)

# Save user CFLAGS and CPPFLAGS before we append internal flags.
override USER_CFLAGS := $(CFLAGS)
override USER_CPPFLAGS := $(CPPFLAGS)

# Internal C flags that should not be changed by the user.
override CFLAGS += \
    -Wall \
    -Wextra \
    -std=gnu11 \
    -nostdinc \
    -ffreestanding \
    -fno-stack-protector \
    -fno-stack-check \
    -fshort-wchar \
    -fPIE \
    -ffunction-sections \
    -fdata-sections


ifeq ($(CC_IS_CLANG),0)
    override CFLAGS += --param=min-pagesize=0
endif

# Internal C preprocessor flags that should not be changed by the user.
override CPPFLAGS := \
    -I src \
    -I nyu-efi/inc \
    -isystem freestnd-c-hdrs \
    $(CPPFLAGS) \
    -MMD \
    -MP

# Internal nasm flags that should not be changed by the user.
override NASMFLAGS += \
    -Wall

# Architecture specific internal flags.
ifeq ($(ARCH),ia32)
    ifeq ($(CC_IS_CLANG),1)
        override CC += \
            -target i386-unknown-none
    endif
    override CFLAGS += \
        -m32 \
        -march=i386 \
        -mno-80387
    override LDFLAGS += \
        -Wl,-m,elf_i386
    override NASMFLAGS += \
        -f elf32
endif
ifeq ($(ARCH),x86_64)
    ifeq ($(CC_IS_CLANG),1)
        override CC += \
            -target x86_64-unknown-none
    endif
    override CFLAGS += \
        -m64 \
        -march=x86-64 \
        -mno-80387 \
        -mno-mmx \
        -mno-sse \
        -mno-sse2 \
        -mno-red-zone
    override LDFLAGS += \
        -Wl,-m,elf_x86_64
    override NASMFLAGS += \
        -f elf64
endif

# Internal linker flags that should not be changed by the user.
override LDFLAGS += \
    -Wl,--build-id=none \
    -nostdlib \
    -pie \
    -z text \
    -z max-page-size=0x1000 \
    -Wl,--gc-sections \
    -Wl,-z,execstack \
    -T nyu-efi/src/elf_$(ARCH)_efi.lds

# Use "find" to glob all *.c, *.S, and *.asm{32,64} files in the tree and obtain the
# object and header dependency file names.
override SRCFILES := $(shell cd src && find -L * -type f | LC_ALL=C sort)
override CFILES := $(filter %.c,$(SRCFILES))
override ASFILES := $(filter %.S,$(SRCFILES))
ifeq ($(ARCH),ia32)
override NASMFILES := $(filter %.asm32,$(SRCFILES))
endif
ifeq ($(ARCH),x86_64)
override NASMFILES := $(filter %.asm64,$(SRCFILES))
endif
override OBJ := $(addprefix obj-$(ARCH)/,$(CFILES:.c=.c.o) $(ASFILES:.S=.S.o))
ifeq ($(ARCH),ia32)
override OBJ += $(addprefix obj-$(ARCH)/,$(NASMFILES:.asm32=.asm32.o))
endif
ifeq ($(ARCH),x86_64)
override OBJ += $(addprefix obj-$(ARCH)/,$(NASMFILES:.asm64=.asm64.o))
endif
override HEADER_DEPS := $(addprefix obj-$(ARCH)/,$(CFILES:.c=.c.d) $(ASFILES:.S=.S.d))

# Default target. This must come first, before header dependencies.
.PHONY: all
all: bin-$(ARCH)/$(OUTPUT).efi

# Include header dependencies.
-include $(HEADER_DEPS)

obj-$(ARCH)/printf.c.o: override CPPFLAGS += \
    -I nanoprintf

# Rules to build the nyu-efi objects we need.
nyu-efi/src/crt0-efi-$(ARCH).S.o: nyu-efi

nyu-efi/src/reloc_$(ARCH).c.o: nyu-efi

.PHONY: nyu-efi
nyu-efi:
	$(MAKE) -C nyu-efi/src -f nyu-efi.mk \
		ARCH="$(ARCH)" \
		CC="$(CC)" \
		CFLAGS="$(USER_CFLAGS) -nostdinc" \
		CPPFLAGS="$(USER_CPPFLAGS) -isystem ../../freestnd-c-hdrs"

# Rule to convert the final ELF executable to a .EFI PE executable.
bin-$(ARCH)/$(OUTPUT).efi: bin-$(ARCH)/$(OUTPUT) GNUmakefile
	mkdir -p "$$(dirname $@)"
	$(OBJCOPY) -O binary $< $@
	dd if=/dev/zero of=$@ bs=4096 count=0 seek=$$(( ($$(wc -c < $@) + 4095) / 4096 )) 2>/dev/null

# Link rules for the final executable.
bin-$(ARCH)/$(OUTPUT): GNUmakefile nyu-efi/src/elf_$(ARCH)_efi.lds nyu-efi/src/crt0-efi-$(ARCH).S.o nyu-efi/src/reloc_$(ARCH).c.o $(OBJ)
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) $(LDFLAGS) nyu-efi/src/crt0-efi-$(ARCH).S.o nyu-efi/src/reloc_$(ARCH).c.o $(OBJ) -o $@

# Compilation rules for *.c files.
obj-$(ARCH)/%.c.o: src/%.c GNUmakefile
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

# Compilation rules for *.S files.
obj-$(ARCH)/%.S.o: src/%.S GNUmakefile
	mkdir -p "$$(dirname $@)"
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

ifeq ($(ARCH),ia32)
# Compilation rules for *.asm32 (nasm) files.
obj-$(ARCH)/%.asm32.o: src/%.asm32 GNUmakefile
	mkdir -p "$$(dirname $@)"
	nasm $(NASMFLAGS) $< -o $@
endif

ifeq ($(ARCH),x86_64)
# Compilation rules for *.asm64 (nasm) files.
obj-$(ARCH)/%.asm64.o: src/%.asm64 GNUmakefile
	mkdir -p "$$(dirname $@)"
	nasm $(NASMFLAGS) $< -o $@
endif

# Rules to download the UEFI firmware per architecture for testing.
ovmf/ovmf-code-$(ARCH).fd:
	mkdir -p ovmf
	curl -Lo $@ https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/ovmf-code-$(ARCH).fd
	case "$(ARCH)" in \
		aarch64) dd if=/dev/zero of=$@ bs=1 count=0 seek=67108864 2>/dev/null;; \
		riscv64) dd if=/dev/zero of=$@ bs=1 count=0 seek=33554432 2>/dev/null;; \
	esac

ovmf/ovmf-vars-$(ARCH).fd:
	mkdir -p ovmf
	curl -Lo $@ https://github.com/osdev0/edk2-ovmf-nightly/releases/latest/download/ovmf-vars-$(ARCH).fd
	case "$(ARCH)" in \
		aarch64) dd if=/dev/zero of=$@ bs=1 count=0 seek=67108864 2>/dev/null;; \
		riscv64) dd if=/dev/zero of=$@ bs=1 count=0 seek=33554432 2>/dev/null;; \
	esac

# Rules for running our executable in QEMU.
.PHONY: run
run: all ovmf/ovmf-code-$(ARCH).fd ovmf/ovmf-vars-$(ARCH).fd
	mkdir -p boot/EFI/BOOT
ifeq ($(ARCH),ia32)
	cp bin-$(ARCH)/$(OUTPUT).efi boot/EFI/BOOT/BOOTIA32.EFI
	qemu-system-$(ARCH) \
		-M q35 \
		-drive if=pflash,unit=0,format=raw,file=ovmf/ovmf-code-$(ARCH).fd,readonly=on \
		-drive if=pflash,unit=1,format=raw,file=ovmf/ovmf-vars-$(ARCH).fd \
		-drive file=fat:rw:boot \
		$(QEMUFLAGS)
endif
ifeq ($(ARCH),x86_64)
	cp bin-$(ARCH)/$(OUTPUT).efi boot/EFI/BOOT/BOOTX64.EFI
	qemu-system-$(ARCH) \
		-M q35 \
		-drive if=pflash,unit=0,format=raw,file=ovmf/ovmf-code-$(ARCH).fd,readonly=on \
		-drive if=pflash,unit=1,format=raw,file=ovmf/ovmf-vars-$(ARCH).fd \
		-drive file=fat:rw:boot \
		$(QEMUFLAGS)
endif
	rm -rf boot

# Remove object files and the final executable.
.PHONY: clean
clean:
	$(MAKE) -C nyu-efi/src -f nyu-efi.mk ARCH="$(ARCH)" clean
	rm -rf bin-$(ARCH) obj-$(ARCH)

# Remove everything built and generated including downloaded dependencies.
.PHONY: distclean
distclean:
	rm -rf bin-* obj-* ovmf
