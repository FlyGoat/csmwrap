#ifndef _CSM_WRAP_H
#define _CSM_WRAP_H

#include <uefi.h>
#include "LegacyBios.h"
#include "x86thunk.h"

#define FALSE   0
#define TRUE    1

struct csmwrap_priv {
    uint8_t *csm_bin;
    uint8_t *vgabios_bin;

    EFI_COMPATIBILITY16_TABLE *csm_efi_table;
    uintptr_t csm_bin_base;
    struct low_stub *low_stub;

    /* VGA stuff */
    uint8_t vga_pci_bus;
    uint8_t vga_devfn;    
    struct csm_vga_table *vga_table;
};

extern int unlock_bios_region();
extern int csmwrap_video_init(struct csmwrap_priv *priv);
extern int copy_rsdt(struct csmwrap_priv *priv);

static inline int
efi_guidcmp (efi_guid_t left, efi_guid_t right)
{
	return memcmp(&left, &right, sizeof (efi_guid_t));
}

#pragma pack(1)
struct low_stub {
    LOW_MEMORY_THUNK thunk;

    EFI_TO_COMPATIBILITY16_INIT_TABLE init_table;
    EFI_TO_COMPATIBILITY16_BOOT_TABLE boot_table;
    EFI_DISPATCH_OPROM_TABLE vga_oprom_table;
};
#pragma pack()

/* Memory map information */
/* In low memory */
#define CONVEN_START    0x00007E00
/* We may have some stack here */
#define LOW_STUB_BASE  0x00020000
/* Thunk + PMM */
#define CONVEN_END      0x00080000
#define EBDA_BASE       CONVEN_END
#define VGABIOS_START   0x000C0000
#define VGABIOS_END     0x000C8000
#define BIOSROM_START   VGABIOS_END
#define BIOSROM_END     0x00100000
/* End of low 1MiB */
#define HIPMM_SIZE      0x400000 /* Allocated on runtime, can be anywhere in 32bit */

#endif
