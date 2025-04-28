#include <uefi.h>

#include "csmwrap.h"

/* 
 * E820 memory map definitions
 * Based on the legacy BIOS E820 memory mapping interface
 */
#define E820_RAM        1
#define E820_RESERVED   2
#define E820_ACPI       3
#define E820_NVS        4
#define E820_UNUSABLE   5

/* 
 * Convert UEFI memory types to E820 types 
 */
static uint32_t convert_memory_type(efi_memory_type_t type)
{
    switch (type) {
        case EfiConventionalMemory:
            return E820_RAM;
        case EfiACPIReclaimMemory:
            return E820_ACPI;
        case EfiACPIMemoryNVS:
            return E820_NVS;
        case EfiUnusableMemory:
            return E820_UNUSABLE;
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
            return E820_RESERVED;
    }
}

/*
 * Build E820 memory map based on UEFI GetMemoryMap
 * Return the number of entries in the E820 map
 */
int build_e820_map(struct csmwrap_priv *priv)
{
    efi_memory_descriptor_t *memory_map = NULL;
    efi_memory_descriptor_t *memory_map_end;
    efi_memory_descriptor_t *memory_map_ptr;
    struct e820_entry *e820_map;
    uintn_t memory_map_size = 0;
    uintn_t map_key = 0;
    uintn_t descriptor_size = 0;
    uint32_t descriptor_version = 0;
    efi_status_t status;
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
    memory_map_end = (efi_memory_descriptor_t *)((uint8_t *)memory_map + memory_map_size);
    
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
        
        /* Try to merge with previous entry if possible */
        if (e820_entries > 0 && 
            e820_map[e820_entries - 1].addr + e820_map[e820_entries - 1].size == start &&
            e820_map[e820_entries - 1].type == type) {
            /* Extend the previous entry */
            e820_map[e820_entries - 1].size += (end - start);
        } else {
            /* Create a new entry */
            e820_map[e820_entries].addr = start;
            e820_map[e820_entries].size = end - start;
            e820_map[e820_entries].type = type;
            e820_entries++;
        }
    }
    
    /* Free the UEFI memory map */
    BS->FreePool(memory_map);
    
    /* Save the number of entries in the low_stub */
    priv->low_stub->e820_entries = e820_entries;
    
    printf("E820 memory map created with %d entries\n", e820_entries);
    
    /* Print the E820 map entries for debugging */
    for (int i = 0; i < e820_entries; i++) {
        printf("E820: [%x-%x] type %d\n",
               (unsigned int) e820_map[i].addr,
               (unsigned int) (e820_map[i].addr + e820_map[i].size - 1),
               e820_map[i].type);
    }
    
    return e820_entries;
}
