#include <efi.h>
#include <printf.h>
#include "csmwrap.h"

uintptr_t g_rsdp = 0;

bool acpi_init(struct csmwrap_priv *priv) {
    UINTN i;
    EFI_GUID acpiGuid = ACPI_TABLE_GUID;
    EFI_GUID acpi2Guid = ACPI_20_TABLE_GUID;
    void *table_target = priv->csm_bin + (priv->csm_efi_table->AcpiRsdPtrPointer - priv->csm_bin_base);

    for (i = 0; i < gST->NumberOfTableEntries; i++) {
        EFI_CONFIGURATION_TABLE *table;
        table = gST->ConfigurationTable + i;

        if (!efi_guidcmp(table->VendorGuid, acpi2Guid)) {
            printf("Found ACPI 2.0 RSDT at %x, copied to %x\n", (uintptr_t)table->VendorTable, (uintptr_t)table_target);
            memcpy(table_target, table->VendorTable, sizeof(EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER));
            g_rsdp = (uintptr_t)table->VendorTable;
            break;
        }
    }

    if (g_rsdp == 0) {
        for (i = 0; i < gST->NumberOfTableEntries; i++) {
            EFI_CONFIGURATION_TABLE *table;
            table = gST->ConfigurationTable + i;

            if (!efi_guidcmp(table->VendorGuid, acpiGuid)) {
                printf("Found ACPI 1.0 RSDT at %x, copied to %x\n", (uintptr_t)table->VendorTable, (uintptr_t)table_target);
                memcpy(table_target, table->VendorTable, sizeof(EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER));
                g_rsdp = (uintptr_t)table->VendorTable;
                break;
            }
        }
    }

    if (g_rsdp) {
        return true;
    }

    printf("No ACPI RSDT found\n");
    return false;
}
