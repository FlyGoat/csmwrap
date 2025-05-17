#ifndef _EDK2COMPAT_H
#define _EDK2COMPAT_H

#include <uefi.h>

#define IN
#define OUT
#define OPTIONAL
#if 0
#define PACKED __attribute__((packed))
#else
#define PACKED
#endif

#define CHAR8 char
#define UINT8 uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define UINT64 uint64_t
#define UINTN uintn_t
#define BOOLEAN boolean_t
#define VOID void

#define SIGNATURE_16(A,B)             EFI_SIGNATURE_16(A,B)
#define SIGNATURE_32(A,B,C,D)         EFI_SIGNATURE_32(A,B,C,D)
#define SIGNATURE_64(A,B,C,D,E,F,G,H) EFI_SIGNATURE_64(A,B,C,D,E,F,G,H)

#define LShiftU64(Value, ShiftCount) ((UINT64)(Value) << (ShiftCount))

#define EFI_STATUS efi_status_t
#define EFI_GUID efi_guid_t
#define GUID efi_guid_t
#define EFI_PHYSICAL_ADDRESS efi_physical_address_t
#define EFI_ALLOCATE_TYPE efi_allocate_type_t
#define EFI_MEMORY_TYPE efi_memory_type_t
#define EFI_HANDLE efi_handle_t


/* Device Path stuff */
#define EFI_DEVICE_PATH_PROTOCOL efi_device_path_t

///
/// Hardware Device Paths.
///
#define HARDWARE_DEVICE_PATH  0x01

///
/// PCI Device Path SubType.
///
#define HW_PCI_DP  0x01

typedef struct {
  EFI_DEVICE_PATH_PROTOCOL    Header;
  ///
  /// PCI Function Number.
  ///
  UINT8                       Function;
  ///
  /// PCI Device Number.
  ///
  UINT8                       Device;
} PCI_DEVICE_PATH;

///
/// BIOS Boot Specification Device Path.
///
#define BBS_DEVICE_PATH  0x05

///
/// BIOS Boot Specification Device Path SubType.
///
#define BBS_BBS_DP  0x01

///
/// This Device Path is used to describe the booting of non-EFI-aware operating systems.
///
typedef struct {
  EFI_DEVICE_PATH_PROTOCOL    Header;
  ///
  /// Device Type as defined by the BIOS Boot Specification.
  ///
  UINT16                      DeviceType;
  ///
  /// Status Flags as defined by the BIOS Boot Specification.
  ///
  UINT16                      StatusFlag;
  ///
  /// Null-terminated ASCII string that describes the boot device to a user.
  ///
  CHAR8                       String[1];
} BBS_BBS_DEVICE_PATH;


#endif
