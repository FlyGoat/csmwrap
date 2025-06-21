#include <efi.h>
#include <printf.h>
#include "csmwrap.h"

#include <uacpi/kernel_api.h>
#include <uacpi/tables.h>
#include <uacpi/uacpi.h>

uintptr_t g_rsdp = 0;

static inline const char *uacpi_log_level_to_string(uacpi_log_level lvl) {
    switch (lvl) {
        case UACPI_LOG_DEBUG:
            return "DEBUG";
        case UACPI_LOG_TRACE:
            return "TRACE";
        case UACPI_LOG_INFO:
            return "INFO";
        case UACPI_LOG_WARN:
            return "WARN";
        case UACPI_LOG_ERROR:
        default:
            return "ERROR";
    }
}

void uacpi_kernel_log(enum uacpi_log_level lvl, const char *text) {
    printf("[uACPI][%s] %s", uacpi_log_level_to_string(lvl), text);
}

void *uacpi_kernel_map(uacpi_phys_addr addr, EFI_UNUSED uacpi_size len) {
    return (void*)((uintptr_t)addr);
}

void uacpi_kernel_unmap(EFI_UNUSED void *ptr, EFI_UNUSED uacpi_size len) {
}

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *rsdp) {
    if (!g_rsdp) {
        return UACPI_STATUS_NOT_FOUND;
    }

    *rsdp = g_rsdp;
    return UACPI_STATUS_OK;
}

static void *early_table_buffer;

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
        const size_t table_buffer_size = 4096;

        if (gBS->AllocatePool(EfiLoaderData, table_buffer_size, &early_table_buffer) != EFI_SUCCESS) {
            return false;
        }

        enum uacpi_status uacpi_status;
        uacpi_status = uacpi_setup_early_table_access(early_table_buffer, table_buffer_size);
        if (uacpi_status != UACPI_STATUS_OK) {
            printf("uACPI early table setup failed: %s\n", uacpi_status_to_string(uacpi_status));
            return false;
        }

        return true;
    }

    printf("No ACPI RSDT found\n");
    return false;
}

void acpi_prepare_exitbs(struct csmwrap_priv *priv) {
    (void)priv;

    if (early_table_buffer != NULL) {
        gBS->FreePool(early_table_buffer);
        early_table_buffer = NULL;
    }
}
