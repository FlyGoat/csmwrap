#include <efi.h>
#include <printf.h>
#include "csmwrap.h"

#include <uacpi/kernel_api.h>
#include <uacpi/tables.h>
#include <uacpi/uacpi.h>

uintptr_t g_rsdp = 0;
void *g_early_table_buf = NULL;

static inline const char *uacpi_log_level_to_string(uacpi_log_level lvl)
{
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

void uacpi_kernel_log(enum uacpi_log_level lvl, const char *text)
{
    printf("[uACPI][%s] %s", uacpi_log_level_to_string(lvl), text);
}

void *uacpi_kernel_map(uacpi_phys_addr addr, EFI_UNUSED uacpi_size len)
{
    return (void*)((uintptr_t)addr);
}

void uacpi_kernel_unmap(EFI_UNUSED void *ptr, EFI_UNUSED uacpi_size len)
{
}

uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *rsdp)
{
    if (!g_rsdp) {
        return UACPI_STATUS_NOT_FOUND;
    }
    *rsdp = g_rsdp;
    return UACPI_STATUS_OK;
}

EFI_STATUS acpi_init(struct csmwrap_priv *priv)
{
    UINTN i;
    EFI_STATUS status;
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

        if (!efi_guidcmp(table->VendorGuid, acpiGuid)) {
            printf("Found ACPI 1.0 RSDT at %x, copied to %x\n", (uintptr_t)table->VendorTable, (uintptr_t)table_target);
            memcpy(table_target, table->VendorTable, sizeof(EFI_ACPI_1_0_ROOT_SYSTEM_DESCRIPTION_POINTER));
            g_rsdp = (uintptr_t)table->VendorTable;
            break;
        }
    }

    if (g_rsdp) {
        const int buffer_size = 4096;
        uacpi_status uacpi_status;
        status = gBS->AllocatePool(EfiLoaderData, buffer_size, (void **)&g_early_table_buf);
        if (EFI_ERROR(status)) {
            return status;
        }
        uacpi_status = uacpi_setup_early_table_access(
            g_early_table_buf, buffer_size
        );
        return (uacpi_status == UACPI_STATUS_OK) ? EFI_SUCCESS : EFI_DEVICE_ERROR;
    }

    printf("No ACPI RSDT found\n");
    return EFI_UNSUPPORTED;
}

EFI_STATUS acpi_prepare_exitbs(EFI_UNUSED struct csmwrap_priv *priv)
{
    if (g_early_table_buf) {
        gBS->FreePool(g_early_table_buf);
        g_early_table_buf = NULL;
    }

    return EFI_SUCCESS;
}
