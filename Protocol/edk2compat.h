#ifndef _EDK2COMPAT_H
#define _EDK2COMPAT_H

#include <uefi.h>

#define IN
#define OUT
#define OPTIONAL

#define UINT8 uint8_t
#define UINT16 uint16_t
#define UINT32 uint32_t
#define BOOLEAN boolean_t

#define SIGNATURE_16(A,B)             EFI_SIGNATURE_16(A,B)
#define SIGNATURE_32(A,B,C,D)         EFI_SIGNATURE_32(A,B,C,D)
#define SIGNATURE_64(A,B,C,D,E,F,G,H) EFI_SIGNATURE_64(A,B,C,D,E,F,G,H)

#define EFI_STATUS efi_status_t
#define EFI_GUID efi_guid_t

#endif
