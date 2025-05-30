#include <efi.h>
#include "csmwrap.h"

#include "io.h"


// See Silicon/ArrowlakePkg/Include/Register/PchBdfAssignment.h
//
// Primary to Sideband (P2SB) Bridge SOC (D31:F1)
//
#define PCI_DEVICE_NUMBER_PCH_P2SB                    31
#define PCI_FUNCTION_NUMBER_PCH_P2SB                  1

#define	SBREG_BAR		0x10
#define SBREG_BARH      0x14

#define PID_ITSS        0xC4

#define R_PCH_PCR_ITSS_ITSSPRC                0x3300          ///< ITSS Power Reduction Control
#define B_PCH_PCR_ITSS_ITSSPRC_PGCBDCGE       (1 << 4)            ///< PGCB Dynamic Clock Gating Enable
#define B_PCH_PCR_ITSS_ITSSPRC_HPETDCGE       (1 << 3)            ///< HPET Dynamic Clock Gating Enable
#define B_PCH_PCR_ITSS_ITSSPRC_8254CGE        (1 << 2)            ///< 8254 Static Clock Gating Enable
#define B_PCH_PCR_ITSS_ITSSPRC_IOSFICGE       (1 << 1)            ///< IOSF-Sideband Interface Clock Gating Enable
#define B_PCH_PCR_ITSS_ITSSPRC_ITSSCGE        (1 << 0)            ///< ITSS Clock Gate Enable

#define PCH_PCR_ADDRESS(Base, Pid, Offset)    ((void *)(Base | (UINT32) (((Offset) & 0x0F0000) << 8) | ((UINT8)(Pid) << 16) | (UINT16) ((Offset) & 0xFFFF)))

#define PORT_PIT_COUNTER0      0x0040
#define PORT_PIT_COUNTER1      0x0041
#define PORT_PIT_COUNTER2      0x0042
#define PORT_PIT_MODE          0x0043
#define PORT_PS2_CTRLB         0x0061

// Bits for PORT_PIT_MODE
#define PM_SEL_TIMER0   (0<<6)
#define PM_SEL_TIMER1   (1<<6)
#define PM_SEL_TIMER2   (2<<6)
#define PM_SEL_READBACK (3<<6)
#define PM_ACCESS_LATCH  (0<<4)
#define PM_ACCESS_LOBYTE (1<<4)
#define PM_ACCESS_HIBYTE (2<<4)
#define PM_ACCESS_WORD   (3<<4)
#define PM_MODE0 (0<<1)
#define PM_MODE1 (1<<1)
#define PM_MODE2 (2<<1)
#define PM_MODE3 (3<<1)
#define PM_MODE4 (4<<1)
#define PM_MODE5 (5<<1)
#define PM_CNT_BINARY (0<<0)
#define PM_CNT_BCD    (1<<0)
#define PM_READ_COUNTER0 (1<<1)
#define PM_READ_COUNTER1 (1<<2)
#define PM_READ_COUNTER2 (1<<3)
#define PM_READ_STATUSVALUE (0<<4)
#define PM_READ_VALUE       (1<<4)
#define PM_READ_STATUS      (2<<4)

#define R_P2SB_CFG_P2SBC                      0x000000e0U      ///< P2SB Control
                                                               /* P2SB general configuration register
                                                                */
#define B_P2SB_CFG_P2SBC_HIDE                 (1 << 8)         ///< P2SB Hide Bit


EFI_STATUS find_p2sb_pci(struct csmwrap_priv *priv, UINT64 *pci_address)
{
    PCI_TYPE00 pdev;

    for (int pciroot = 0; pciroot < priv->rootbus_count; pciroot++) {
        UINT8 bus = priv->rootbus_list[pciroot];
        UINT64 d31f0 = EFI_PCI_ADDRESS(bus, 31, 0);

        if (PciConfigRead(EfiPciIoWidthUint32, d31f0, sizeof(pdev.Hdr) / 4, &pdev.Hdr) != EFI_SUCCESS) {
            continue; // Skip if configuration cannot be read
        }

        if (pdev.Hdr.VendorId != 0x8086 || !IS_PCI_MULTI_FUNC(&pdev)) {
            continue; // Not an Intel multifunc device
        }

        if (!IS_CLASS2(&pdev, PCI_CLASS_BRIDGE, PCI_CLASS_BRIDGE_ISA)) {
            continue; // D31F0 is not a ISA bridge
        }

        *pci_address = EFI_PCI_ADDRESS(bus, PCI_DEVICE_NUMBER_PCH_P2SB, PCI_FUNCTION_NUMBER_PCH_P2SB);

        return EFI_SUCCESS; // Found P2SB
    }

    return EFI_NOT_FOUND; // No P2SB found
}

static int pit_8254cge_workaround(struct csmwrap_priv *priv)
{
    EFI_STATUS Status;
    uint32_t reg;
    unsigned long base;
    bool p2sb_hide = false;
    UINT64 p2sb_addr;

    Status = find_p2sb_pci(priv, &p2sb_addr);
    if (EFI_ERROR(Status)) {
        DEBUG((DEBUG_INFO, "P2SB Not found %d, skipping 8254CGE workaround\n", Status));
        return -1; // P2SB not found, skip workaround
    }

    DEBUG((DEBUG_INFO, "P2SB Address: %lx\n", p2sb_addr));

    Status = PciConfigRead(EfiPciIoWidthUint32, p2sb_addr, 1, &reg);

    /* P2SB maybe hidden, try unhide it first */
    if ((reg & 0xFFFF) == 0xffff) {
        Status = PciConfigRead(EfiPciIoWidthUint32, p2sb_addr + R_P2SB_CFG_P2SBC, 1, &reg);
        reg &= ~B_P2SB_CFG_P2SBC_HIDE;
        Status = PciConfigWrite(EfiPciIoWidthUint32, p2sb_addr + R_P2SB_CFG_P2SBC, 1, &reg);
        p2sb_hide = true;
    }

    /* Read header again */
    Status = PciConfigRead(EfiPciIoWidthUint32, p2sb_addr, 1, &reg);

    if ((reg & 0xFFFF) != 0x8086) {
        DEBUG((DEBUG_ERROR, "Unable to unhide P2SB\n"));
        goto test_pit;
    }

    Status = PciConfigRead(EfiPciIoWidthUint32, p2sb_addr + SBREG_BAR, 1, &reg);
    base = reg & ~0x0F;

    Status = PciConfigRead(EfiPciIoWidthUint32, p2sb_addr + SBREG_BARH, 1, &reg);
#ifdef __LP64__
    base |= ((uint64_t)reg & 0xFFFFFFFF) << 32;
#else
    if (reg) {
        printf("Invalid P2SB BARH\n");
        goto test_pit;
    }
#endif
    /* FIXME: Validate base ? */
    reg = readl(PCH_PCR_ADDRESS(base, PID_ITSS, R_PCH_PCR_ITSS_ITSSPRC));
    printf("ITSSPRC = %x, ITSSPRC.8254CGE= %d\n", reg, !!(reg & B_PCH_PCR_ITSS_ITSSPRC_8254CGE));
    /* Disable 8254CGE */
    reg &= ~B_PCH_PCR_ITSS_ITSSPRC_8254CGE;
    writel(PCH_PCR_ADDRESS(base, PID_ITSS, R_PCH_PCR_ITSS_ITSSPRC), reg);

    /* Hide P2SB again */
    if (p2sb_hide) {
        Status = PciConfigRead(EfiPciIoWidthUint32, p2sb_addr + R_P2SB_CFG_P2SBC, 1, &reg);
        reg |= ~B_P2SB_CFG_P2SBC_HIDE;
        Status = PciConfigWrite(EfiPciIoWidthUint32, p2sb_addr + R_P2SB_CFG_P2SBC, 1, &reg);
    }

test_pit:
    /* Lets hope we will not BOOM UEFI with this */
    outb(PORT_PIT_MODE, PM_SEL_READBACK | PM_READ_VALUE | PM_READ_COUNTER0);
    uint16_t v1 = inb(PORT_PIT_COUNTER0) | (inb(PORT_PIT_COUNTER0) << 8);

    gBS->Stall(1000);
    outb(PORT_PIT_MODE, PM_SEL_READBACK | PM_READ_VALUE | PM_READ_COUNTER0);
    uint16_t v2 = inb(PORT_PIT_COUNTER0) | (inb(PORT_PIT_COUNTER0) << 8);
    if (v1 == v2) {
        printf("PIT test failed, not counting!\n");
        return -1;
    }

    return 0;
}

EFI_STATUS apply_intel_platform_workarounds(struct csmwrap_priv *priv)
{
    if (priv->hbridge_hdr.VendorId != 0x8086) {
        return EFI_SUCCESS; // Not an Intel platform
    }

    pit_8254cge_workaround(priv);

    return EFI_SUCCESS;
}
