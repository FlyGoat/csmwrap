# CSMWrap

CSMWrap is a cool little hack that brings back the good old PC BIOS on those fancy-pants UEFI-only systems. It utilises the CSM (Compatibility Support Module) and VESA VBIOS from SeaBIOS Project to emulate a legacy BIOS environment.

## Current Status

Right now, CSMWrap can:

- Boot FreeDOS, Windows XP, and Windows 7 in QEMU (both q35 and piix4 machines)
- Run on some real hardware too! (Your mileage may vary)

## Implementation Details

CSMWrap works by:

- Unlocking the legacy BIOS memory region (0xC0000-0xFFFFF)
- Loading the SeaBIOS CSM module into memory
- Configuring memory mapping for legacy applications
- Setting up VGA BIOS with information from EFI GOP
- Building an E820 memory map based on EFI memory map
- Providing essential compatibility tables (ACPI, SMBIOS)
- Initializing the CSM module and legacy services
- Transferring control to the legacy boot process

## How to Use

Simply use `csmwarp.efi` as your bootloader, you can place it in your EFI partition and boot from it. Remember to disable Secure Boot, and Above 4G Decoding in your BIOS/UEFI settings.

## Limitations
### Above 4G Decoding

It is almost required to run CSMWrap with above 4G decoding disabled in your BIOS/UEFI. As UEFI firmwares are likely to place GPU's VRAM BAR above 4G, and legacy BIOS are 32bit which means it can only access the first 4G of memory.

### Legacy Region Unlocking

Currently csmwrap relies on `EFI_LEGACY_REGION2_PROTOCOL` to enable writing to the legacy region. This is not available on all systems. For system that do not support this protocol, csmwrap will attempt to use PAM registers in chipset to perform decoding, which is not guaranteed to work.

### Windows Video Modesetting Issues

Windows XP/7's video modesetting logic is a bit mysterious. It may try to set a incompatible mode using `int10h`, which will cause flickering or even black screen after transferring control to the legacy OS.

This is a known issue and may be fixed in the future.

Meanwhile you can try to inject the GPU driver to OS image to avoid using the VESA BIOS.

## Credits
- [SeaBIOS](https://www.seabios.org/) for the CSM module and VESA VBIOS
- [Nyu-EFI](https://codeberg.org/osdev/nyu-efi) for EFI C runtime, build system, and headers
- [EDK2](https://github.com/tianocore/edk2) for code snippets
- @CanonKong for test feedback and general knowledge
