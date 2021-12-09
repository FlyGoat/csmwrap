#include <uefi.h>
#include "csmwrap.h"

struct RSDPDescriptor {
 char Signature[8];
 uint8_t Checksum;
 char OEMID[6];
 uint8_t Revision;
 uint32_t RsdtAddress;
} __attribute__ ((packed));

struct RSDPDescriptor20 {
 struct RSDPDescriptor firstPart;
 
 uint32_t Length;
 uint64_t XsdtAddress;
 uint8_t ExtendedChecksum;
 uint8_t reserved[3];
} __attribute__ ((packed));

int copy_rsdt(struct csmwrap_priv *priv)
{
    int i;
    efi_guid_t acpiGuid = ACPI_TABLE_GUID;
    efi_guid_t acpi2Guid = ACPI_20_TABLE_GUID;
    void *table_target = priv->csm_bin + (priv->csm_efi_table->AcpiRsdPtrPointer - priv->csm_bin_base);


    for (i = 0; i < ST->NumberOfTableEntries; i++) {
        efi_configuration_table_t *table;
        table = ST->ConfigurationTable + i;

        if (efi_guidcmp(table->VendorGuid, acpi2Guid)) {
            printf("Found ACPI 2.0 RSDT at %x\n", (uintptr_t)table->VendorTable);
            memcpy(table_target, table->VendorTable, sizeof(struct RSDPDescriptor20));
            return 0;
        }

        if (efi_guidcmp(table->VendorGuid, acpiGuid)) {
            printf("Found ACPI 1.0 RSDT at %x\n", (uintptr_t)table->VendorTable);
            memcpy(table_target, table->VendorTable, sizeof(struct RSDPDescriptor));
            return 0;
        }

    }
    printf("No ACPI RSDT found\n");
    return -1;
}
