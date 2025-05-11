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

#endif
