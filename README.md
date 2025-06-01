# CSMWrap [![Build Status](https://github.com/FlyGoat/CSMWrap/actions/workflows/build.yml/badge.svg)](https://github.com/FlyGoat/CSMWrap/actions/workflows/build.yml)

CSMWrap is a cool UEFI application designed to enable legacy BIOS booting on modern UEFI-only systems. It achieves this by leveraging the Compatibility Support Module (CSM) and VESA VBIOS components from the [SeaBIOS project](https://www.seabios.org/), effectively emulating a traditional PC BIOS environment.

This project aims to bridge the gap for users and developers who need to run operating systems or software that strictly require a legacy BIOS, on hardware that no longer provides one natively.

## Features

*   **Legacy BIOS Emulation:** Provides INT 10h (video), INT 13h (disk), and other essential BIOS interrupts.
*   **SeaBIOS CSM Integration:** Utilizes a specially compiled SeaBIOS CSM (`Csm16.bin`) for core compatibility logic.
*   **SeaVGABIOS for Video:** Includes SeaVGABIOS for VESA VBE support, initialized using information from EFI GOP.
*   **E820 Memory Mapping:** Constructs an E820 memory map compatible with legacy OSes, derived from the UEFI memory map.
*   **ACPI & SMBIOS Passthrough:** Attempts to provide necessary ACPI tables (RSDP) and SMBIOS information to the legacy environment.
*   **Coreboot Table Generation:** Can generate a Coreboot table for payloads that might expect it.
*   **Cross-Architecture Support:** Builds for both IA32 (x86) and x86_64 UEFI systems.
*   **Open Source:** Based on Nyu-EFI, uACPI, and other open components.

## Current Status

CSMWrap has been successfully tested to:
*   Boot FreeDOS.
*   Boot Windows XP and Windows 7 in QEMU (q35 and PIIX4 machine types).
*   Run on some real hardware (compatibility varies).

**Disclaimer:** This project is a work-in-progress. While it aims to provide robust legacy support, hardware and firmware variations can significantly impact its functionality. Success on your specific system is not guaranteed.

## Crucial UEFI Prerequisites

Before attempting to use CSMWrap, your system's UEFI firmware settings **MUST** be configured as follows:

1.  **Secure Boot MUST be DISABLED.**
    *   CSMWrap is not signed and performs low-level operations incompatible with Secure Boot.
2.  **"Above 4G Decoding" / "Resizable BAR" / "Smart Access Memory" MUST be DISABLED.**
    *   Legacy VBIOS, Option ROMs, and 32-bit operating systems generally cannot access PCI device memory (BARs) mapped above the 4GB physical address range. Enabling these features will likely lead to display issues or boot failures with CSMWrap.
3.  **Native CSM (Compatibility Support Module):**
    *   **Try disabling it first.** CSMWrap aims to be its own, self-contained CSM. Your motherboard's native CSM might conflict.
    *   **If CSMWrap fails,** you can try enabling the native CSM. Some motherboards might require their CSM to be enabled for certain very low-level hardware initializations, even if CSMWrap then tries to take over. This requires experimentation.
4.  **SATA Controller Mode:** For older OSes like Windows XP, consider setting your SATA controller to **IDE/Compatible** mode if AHCI drivers are an issue.
5.  **USB Legacy Support:** Ensure this is **ENABLED** if you plan to use USB keyboards/mice in the legacy OS.

Failure to correctly configure these settings is the most common reason for CSMWrap not working.

## Quick Start

1.  **Get CSMWrap:**
    *   Download a pre-compiled `.efi` file (`csmwrapx64.efi` for 64-bit UEFI or `csmwrapia32.efi` for 32-bit UEFI) from the [Releases page](https://github.com/FlyGoat/CSMWrap/releases).
    *   Alternatively, [Building from Source](https://github.com/FlyGoat/CSMWrap/wiki/Building-from-Source).
2.  **Configure UEFI System:**
    *   Ensure all **Crucial UEFI Prerequisites** listed above are correctly set in your firmware.
3.  **Prepare EFI System Partition (ESP):**
    *   Copy the appropriate `csmwrap<ARCH>.efi` file to your EFI System Partition (ESP, usually a FAT32 partition).
    *   A common path for automatic UEFI boot is `EFI/BOOT/BOOTX64.EFI` (for 64-bit) or `EFI/BOOT/BOOTIA32.EFI` (for 32-bit).
    *   Alternatively, place it elsewhere on the ESP (e.g., `EFI/CSMWrap/csmwrap.efi`) and create a custom UEFI boot entry.
4.  **Boot:**
    *   Restart your computer and select the UEFI boot entry for CSMWrap.
    *   CSMWrap will initialize, and you should see the SeaBIOS boot screen, attempting to boot your legacy OS.

For more detailed instructions, see the [Usage Guide](https://github.com/FlyGoat/CSMWrap/wiki/Usage-Guide) and [Partitioning and Boot Scenarios](https://github.com/FlyGoat/CSMWrap/wiki/Partitioning-and-Boot-Scenarios).

## Building from Source

If you wish to compile CSMWrap yourself, please refer to the [Building from Source](https://github.com/FlyGoat/CSMWrap/wiki/Building-from-Source) guide.

## Limitations and Known Issues

CSMWrap is complex and interacts with diverse hardware and firmware. It has known limitations:
*   The "Above 4G Decoding" requirement is fundamental.
*   Unlocking the legacy BIOS memory region (0xC0000-0xFFFFF) can fail on some systems.
*   Windows XP/7 may have video modesetting issues after the initial boot logo.
*   Hardware and firmware compatibility is not universal.

Please see the detailed [Limitations and Known Issues](https://github.com/FlyGoat/CSMWrap/wiki/Limitations-and-Known-Issues) page for more information.

## Documentation

This repository contains a wiki. Please visit [this](https://github.com/FlyGoat/CSMWrap/wiki).

## Troubleshooting

If you encounter problems, please consult the [Troubleshooting](https://github.com/FlyGoat/CSMWrap/wiki/Troubleshooting) guide first. If your issue isn't covered, feel free to open an issue.

## Contributing

Contributions are welcome! Whether it's reporting bugs, suggesting features, improving documentation, or submitting code changes, your help is appreciated.
Please read the [Contributing](https://github.com/FlyGoat/CSMWrap/wiki/Contributing) guide for more details.

## Credits & Acknowledgements

*   The **[SeaBIOS project](https://www.seabios.org/)** for their invaluable CSM and VBIOS code.
*   **[Nyu-EFI](https://codeberg.org/osdev/nyu-efi)** for the EFI C runtime, build system, and headers.
*   **[EDK2 (TianoCore)](https://github.com/tianocore/edk2)** for UEFI specifications and some code inspiration.
*   **[uACPI](https://github.com/uACPI/uACPI)** for ACPI table handling.
*   **@CanonKong** for test feedback and general knowledge.
*   All contributors and testers from the community!
