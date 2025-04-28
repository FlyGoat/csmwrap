/*
 * legacy_region2.h
 *
 * The Legacy Region Protocol controls the read, write and boot-lock attributes for
 * the region 0xC0000 to 0xFFFFF.
 *
 * Copyright (c) 2009 - 2018, Intel Corporation. All rights reserved.
 * This program and the accompanying materials
 * are licensed and made available under the terms and conditions of the BSD License
 * which accompanies this distribution.  The full text of the license may be found at
 * http://opensource.org/licenses/bsd-license.php
 *
 * THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 * WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
 *
 * Revision Reference:
 * This Protocol is defined in UEFI Platform Initialization Specification 1.2
 * Volume 5: Standards
 */

 #ifndef _LEGACY_REGION2_H_
 #define _LEGACY_REGION2_H_
 
 #define EFI_LEGACY_REGION2_PROTOCOL_GUID \
     { 0x70101eaf, 0x85, 0x440c, {0xb3, 0x56, 0x8e, 0xe3, 0x6f, 0xef, 0x24, 0xf0} }
 
 typedef struct _efi_legacy_region2_protocol efi_legacy_region2_protocol_t;
 
 /**
  * Modify the hardware to allow (decode) or disallow (not decode) memory reads in a region.
  *
  * If the On parameter evaluates to TRUE, this function enables memory reads in the address range
  * Start to (Start + Length - 1).
  * If the On parameter evaluates to FALSE, this function disables memory reads in the address range
  * Start to (Start + Length - 1).
  *
  * @param  This[in]              Indicates the efi_legacy_region2_protocol_t instance.
  * @param  Start[in]             The beginning of the physical address of the region whose attributes
  *                               should be modified.
  * @param  Length[in]            The number of bytes of memory whose attributes should be modified.
  *                               The actual number of bytes modified may be greater than the number
  *                               specified.
  * @param  Granularity[out]      The number of bytes in the last region affected. This may be less
  *                               than the total number of bytes affected if the starting address
  *                               was not aligned to a region's starting address or if the length
  *                               was greater than the number of bytes in the first region.
  * @param  On[in]                Decode / Non-Decode flag.
  *
  * @retval EFI_SUCCESS           The region's attributes were successfully modified.
  * @retval EFI_INVALID_PARAMETER If Start or Length describe an address not in the Legacy Region.
  */
 typedef
 efi_status_t
 (EFIAPI *efi_legacy_region2_decode_t)(
     efi_legacy_region2_protocol_t  *This,
     uint32_t                       Start,
     uint32_t                       Length,
     uint32_t                       *Granularity,
     boolean_t                      *On
 );
 
 /**
  * Modify the hardware to disallow memory writes in a region.
  *
  * This function changes the attributes of a memory range to not allow writes.
  *
  * @param  This[in]              Indicates the efi_legacy_region2_protocol_t instance.
  * @param  Start[in]             The beginning of the physical address of the region whose
  *                               attributes should be modified.
  * @param  Length[in]            The number of bytes of memory whose attributes should be modified.
  *                               The actual number of bytes modified may be greater than the number
  *                               specified.
  * @param  Granularity[out]      The number of bytes in the last region affected. This may be less
  *                               than the total number of bytes affected if the starting address was
  *                               not aligned to a region's starting address or if the length was
  *                               greater than the number of bytes in the first region.
  *
  * @retval EFI_SUCCESS           The region's attributes were successfully modified.
  * @retval EFI_INVALID_PARAMETER If Start or Length describe an address not in the Legacy Region.
  */
 typedef
 efi_status_t
 (EFIAPI *efi_legacy_region2_lock_t)(
     efi_legacy_region2_protocol_t  *This,
     uint32_t                       Start,
     uint32_t                       Length,
     uint32_t                       *Granularity
 );
 
 /**
  * Modify the hardware to disallow memory attribute changes in a region.
  *
  * This function makes the attributes of a region read only. Once a region is boot-locked with this
  * function, the read and write attributes of that region cannot be changed until a power cycle has
  * reset the boot-lock attribute. Calls to Decode(), Lock() and Unlock() will have no effect.
  *
  * @param  This[in]              Indicates the efi_legacy_region2_protocol_t instance.
  * @param  Start[in]             The beginning of the physical address of the region whose
  *                               attributes should be modified.
  * @param  Length[in]            The number of bytes of memory whose attributes should be modified.
  *                               The actual number of bytes modified may be greater than the number
  *                               specified.
  * @param  Granularity[out]      The number of bytes in the last region affected. This may be less
  *                               than the total number of bytes affected if the starting address was
  *                               not aligned to a region's starting address or if the length was
  *                               greater than the number of bytes in the first region.
  *
  * @retval EFI_SUCCESS           The region's attributes were successfully modified.
  * @retval EFI_INVALID_PARAMETER If Start or Length describe an address not in the Legacy Region.
  * @retval EFI_UNSUPPORTED       The chipset does not support locking the configuration registers in
  *                               a way that will not affect memory regions outside the legacy memory
  *                               region.
  */
 typedef
 efi_status_t
 (EFIAPI *efi_legacy_region2_boot_lock_t)(
     efi_legacy_region2_protocol_t  *This,
     uint32_t                       Start,
     uint32_t                       Length,
     uint32_t                       *Granularity
 );
 
 /**
  * Modify the hardware to allow memory writes in a region.
  *
  * This function changes the attributes of a memory range to allow writes.
  *
  * @param  This[in]              Indicates the efi_legacy_region2_protocol_t instance.
  * @param  Start[in]             The beginning of the physical address of the region whose
  *                               attributes should be modified.
  * @param  Length[in]            The number of bytes of memory whose attributes should be modified.
  *                               The actual number of bytes modified may be greater than the number
  *                               specified.
  * @param  Granularity[out]      The number of bytes in the last region affected. This may be less
  *                               than the total number of bytes affected if the starting address was
  *                               not aligned to a region's starting address or if the length was
  *                               greater than the number of bytes in the first region.
  *
  * @retval EFI_SUCCESS           The region's attributes were successfully modified.
  * @retval EFI_INVALID_PARAMETER If Start or Length describe an address not in the Legacy Region.
  */
 typedef
 efi_status_t
 (EFIAPI *efi_legacy_region2_unlock_t)(
     efi_legacy_region2_protocol_t  *This,
     uint32_t                       Start,
     uint32_t                       Length,
     uint32_t                       *Granularity
 );
 
 typedef enum {
     legacy_region_decoded,         /* This region is currently set to allow reads. */
     legacy_region_not_decoded,     /* This region is currently set to not allow reads. */
     legacy_region_write_enabled,   /* This region is currently set to allow writes. */
     legacy_region_write_disabled,  /* This region is currently set to write protected. */
     legacy_region_boot_locked,     /* This region's attributes are locked, cannot be modified until after a power cycle. */
     legacy_region_not_locked       /* This region's attributes are not locked. */
 } efi_legacy_region_attribute_t;
 
 typedef struct {
     /*
      * The beginning of the physical address of this region.
      */
     uint32_t                      start;
     /*
      * The number of bytes in this region.
      */
     uint32_t                      length;
     /*
      * Attribute of the Legacy Region Descriptor that describes the capabilities for that memory region.
      */
     efi_legacy_region_attribute_t attribute;
     /*
      * Describes the byte length programmability associated with the Start address and the specified
      * Attribute setting.
      */
     uint32_t                      granularity;
 } efi_legacy_region_descriptor_t;
 
 /**
  * Get region information for the attributes of the Legacy Region.
  *
  * This function is used to discover the granularity of the attributes for the memory in the legacy
  * region. Each attribute may have a different granularity and the granularity may not be the same
  * for all memory ranges in the legacy region.
  *
  * @param  This[in]              Indicates the efi_legacy_region2_protocol_t instance.
  * @param  DescriptorCount[out]  The number of region descriptor entries returned in the Descriptor
  *                               buffer.
  * @param  Descriptor[out]       A pointer to a pointer used to return a buffer where the legacy
  *                               region information is deposited. This buffer will contain a list of
  *                               DescriptorCount number of region descriptors. This function will
  *                               provide the memory for the buffer.
  *
  * @retval EFI_SUCCESS           The information structure was returned.
  * @retval EFI_UNSUPPORTED       This function is not supported.
  */
 typedef
 efi_status_t
 (EFIAPI *efi_legacy_region_get_info_t)(
     efi_legacy_region2_protocol_t   *This,
     uint32_t                        *DescriptorCount,
     efi_legacy_region_descriptor_t  **Descriptor
 );
 
 /*
  * The efi_legacy_region2_protocol_t is used to abstract the hardware control of the memory
  * attributes of the Option ROM shadowing region, 0xC0000 to 0xFFFFF.
  * There are three memory attributes that can be modified through this protocol: read, write and
  * boot-lock. These protocols may be set in any combination.
  */
 struct _efi_legacy_region2_protocol {
     efi_legacy_region2_decode_t     decode;
     efi_legacy_region2_lock_t       lock;
     efi_legacy_region2_boot_lock_t  boot_lock;
     efi_legacy_region2_unlock_t     unlock;
     efi_legacy_region_get_info_t    get_info;
 };
 
 extern efi_guid_t gEfiLegacyRegion2ProtocolGuid;
 
 #endif /* _LEGACY_REGION2_H_ */
