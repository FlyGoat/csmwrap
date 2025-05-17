#include <efi.h>

#include <printf.h>
#include "csmwrap.h"

/* This is not in E820.h */
#define EfiAcpiAddressRangeUnusable 5

/*
 * Convert UEFI memory types to E820 types 
 */
static uint32_t convert_memory_type(EFI_MEMORY_TYPE type)
{
    switch (type) {
        case EfiConventionalMemory:
            return EfiAcpiAddressRangeMemory;
        case EfiACPIReclaimMemory:
            return EfiAcpiAddressRangeACPI;
        case EfiACPIMemoryNVS:
            return EfiAcpiAddressRangeNVS;
        case EfiUnusableMemory:
            return EfiAcpiAddressRangeUnusable;
        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
        case EfiMemoryMappedIO:
        case EfiMemoryMappedIOPortSpace:
        case EfiPalCode:
        /* FIXME Those should be usable ? */
        case EfiLoaderCode:
        case EfiLoaderData:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiReservedMemoryType:
        default:
            return EfiAcpiAddressRangeReserved;
    }
}

/*
 * Build E820 memory map based on UEFI GetMemoryMap
 * Return the number of entries in the E820 map
 */
int build_e820_map(struct csmwrap_priv *priv)
{
    EFI_MEMORY_DESCRIPTOR *memory_map = NULL;
    EFI_MEMORY_DESCRIPTOR *memory_map_end;
    EFI_MEMORY_DESCRIPTOR *memory_map_ptr;
    EFI_E820_ENTRY64 *e820_map;
    UINTN memory_map_size = 0;
    UINTN map_key = 0;
    UINTN descriptor_size = 0;
    uint32_t descriptor_version = 0;
    EFI_STATUS status;
    uint32_t e820_entries = 0;
    
    /* First call to get the buffer size */
    status = BS->GetMemoryMap(&memory_map_size, NULL, &map_key, &descriptor_size, &descriptor_version);
    if (status != EFI_BUFFER_TOO_SMALL) {
        printf("Unexpected GetMemoryMap status: %lx\n", status);
        return -1;
    }
    
    /* Add some extra space to handle potential changes between calls */
    memory_map_size += descriptor_size * 10;
    
    /* Allocate memory for the UEFI memory map */
    status = BS->AllocatePool(EfiLoaderData, memory_map_size, (void **)&memory_map);
    if (EFI_ERROR(status)) {
        printf("Failed to allocate memory for memory map: %lx\n", status);
        return -1;
    }
    
    /* Get the actual memory map */
    status = BS->GetMemoryMap(&memory_map_size, memory_map, &map_key, &descriptor_size, &descriptor_version);
    if (EFI_ERROR(status)) {
        printf("Failed to get memory map: %lx\n", status);
        BS->FreePool(memory_map);
        return -1;
    }
    
    /* Initialize the E820 map in the low_stub */
    e820_map = priv->low_stub->e820_map;
    memory_map_end = (EFI_MEMORY_DESCRIPTOR *)((uint8_t *)memory_map + memory_map_size);
    
    /* Process each memory descriptor and convert to E820 format */
    for (memory_map_ptr = memory_map; 
         memory_map_ptr < memory_map_end && e820_entries < E820_MAX_ENTRIES;
         memory_map_ptr = NextMemoryDescriptor(memory_map_ptr, descriptor_size)) {
        
        uint64_t start = memory_map_ptr->PhysicalStart;
        uint64_t end = start + (memory_map_ptr->NumberOfPages * EFI_PAGE_SIZE);
        uint32_t type = convert_memory_type(memory_map_ptr->Type);
        
        /* Skip zero-length regions */
        if (start == end)
            continue;

        /* Skip memory types that are not reported in E820 */
        if (type == 0)
            continue;

        /* Try to merge with previous entry if possible */
        if (e820_entries > 0 && 
            e820_map[e820_entries - 1].BaseAddr + e820_map[e820_entries - 1].Length == start &&
            e820_map[e820_entries - 1].Type == type) {
            /* Extend the previous entry */
            e820_map[e820_entries - 1].Length += (end - start);
        } else {
            /* Create a new entry */
            e820_map[e820_entries].BaseAddr = start;
            e820_map[e820_entries].Length = end - start;
            e820_map[e820_entries].Type = type;
            e820_entries++;
        }
    }
    
    /* Free the UEFI memory map */
    BS->FreePool(memory_map);
    
    /* Save the number of entries in the low_stub */
    priv->low_stub->e820_entries = e820_entries;

#if 0
    printf("E820 memory map created with %d entries\n", e820_entries);
    
    /* Print the E820 map entries for debugging */
    for (int i = 0; i < e820_entries; i++) {
        printf("E820: [%x-%x] type %d\n",
               (unsigned int) e820_map[i].BaseAddr,
               (unsigned int) (e820_map[i].BaseAddr + e820_map[i].Type - 1),
               e820_map[i].Length);
    }
#endif
    
    return e820_entries;
}
