#include <efi.h>
#include <csmwrap.h>
#include <io.h>
#include <uacpi/uacpi.h>
#include <uacpi/tables.h>
#include <uacpi/acpi.h>

/*
 * We only support segment 0 at the moment.
 * This is sufficient for the CSM wrapper, as legacy BIOS can
 * only support segment 0 anyway.
 */

static UINTN mmcfg_maxbus = 0xff;
static void *mmcfg = NULL;

#define PCI_CONFIG_ADDRESS 0xcf8
#define PCI_CONFIG_DATA 0xcfc

static inline void pciSetAddress(unsigned int bus, unsigned int slot,
                   unsigned int function, unsigned int offset)
{
    uint32_t address;

    /* Address bits (inclusive):
     * 31      Enable bit (must be 1 for it to work)
     * 30 - 24 Reserved
     * 23 - 16 Bus number
     * 15 - 11 Slot number
     * 10 - 8  Function number (for multifunction devices)
     * 7 - 2   Register number (offset / 4)
     * 1 - 0   Must always be 00 */
    address = 0x80000000 | ((unsigned long) (bus & 0xff) << 16)
              | ((unsigned long) (slot & 0x1f) << 11)
              | ((unsigned long) (function & 0x7) << 8)
              | ((unsigned long) offset & 0xff);
    /* Full DWORD write to port must be used for PCI to detect new address. */
    outl(PCI_CONFIG_ADDRESS, address);
}

static inline uint8_t pciConfigReadByte(unsigned int bus, unsigned int slot,
                                unsigned int function, unsigned int offset)
{
    pciSetAddress(bus, slot, function, offset);
    /* The PCI registers are little endian,
     * so the last byte of DWORD is read
     * when offset is 0. */
    return (inb(PCI_CONFIG_DATA + (offset & 3)));
}

static inline uint16_t pciConfigReadWord(unsigned int bus, unsigned int slot,
                               unsigned int function, unsigned int offset)
{
    pciSetAddress(bus, slot, function, offset);
    /* The PCI registers are little endian,
     * so the last word of DWORD is read
     * when offset is 0. */
    return (inw(PCI_CONFIG_DATA + (offset & 2)));
}

static inline uint32_t pciConfigReadDWord(unsigned int bus, unsigned int slot,
                                 unsigned int function, unsigned int offset)
{
    pciSetAddress(bus, slot, function, offset);
    return (inl(PCI_CONFIG_DATA));
}

static inline void pciConfigWriteByte(unsigned int bus, unsigned int slot,
                        unsigned int function, unsigned int offset,
                        uint8_t data)
{
    pciSetAddress(bus, slot, function, offset);
    /* The PCI registers are little endian,
     * so the last byte of DWORD is written
     * when offset is 0. */
    outb(PCI_CONFIG_DATA + (offset & 3), data);
}

static inline void pciConfigWriteWord(unsigned int bus, unsigned int slot,
                        unsigned int function, unsigned int offset,
                        uint16_t data)
{
    pciSetAddress(bus, slot, function, offset);
    /* The PCI registers are little endian,
     * so the last word of DWORD is written
     * when offset is 0. */
    outw(PCI_CONFIG_DATA + (offset & 2), data);
}

static inline void pciConfigWriteDWord(unsigned int bus, unsigned int slot,
                         unsigned int function, unsigned int offset,
                         uint32_t data)
{
    pciSetAddress(bus, slot, function, offset);
    outl(PCI_CONFIG_DATA, data);
}

static inline void pciConfigAccessMMCFG(unsigned int bus, unsigned int slot,
                                unsigned int function, unsigned int offset,
                                EFI_PCI_IO_PROTOCOL_WIDTH width,
                                void *data, bool write)
{
    if (mmcfg == NULL) {
        DEBUG((DEBUG_ERROR, "MMCFG not initialized\n"));
        return;
    }

    if (bus > mmcfg_maxbus) {
        DEBUG((DEBUG_ERROR, "Bus number %u exceeds max %u\n", bus, mcfg_maxbus));
        return;
    }

    void *address = mmcfg + EFI_PCI_ADDRESS(bus, slot, function) + offset;

    if (write) {
        switch (width) {
            case EfiPciIoWidthUint8:
                *(uint8_t *)data = readb(address);
                break;
            case EfiPciIoWidthUint16:
                *(uint16_t *)data = readw(address);
                break;
            case EfiPciIoWidthUint32:
                *(uint32_t *)data = readl(address);
                break;
            case EfiPciIoWidthUint64:
                *(uint64_t *)data = readq(address);
                break;
            default:
                DEBUG((DEBUG_ERROR, "Unsupported width %d\n", width));
        }
    } else {
        switch (width) {
            case EfiPciIoWidthUint8:
                writeb(address, *(uint8_t *)data);
                break;
            case EfiPciIoWidthUint16:
                writew(address, *(uint16_t *)data);
                break;
            case EfiPciIoWidthUint32:
                writel(address, *(uint32_t *)data);
                break;
            case EfiPciIoWidthUint64:
                writeq(address, *(uint64_t *)data);
                break;
            default:
                DEBUG((DEBUG_ERROR, "Unsupported width %d\n", width));
        }
    }
}

EFI_STATUS PciConfigRead(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH Width, UINT64 Address,
                         UINTN Count, VOID *Buffer)
{
    UINTN width_bytes;
    UINT8 bus = (UINT8)(Address >> 24) & 0xff;
    UINT8 dev = (UINT8)(Address >> 16) & 0xff;
    UINT8 function = (UINT8)(Address >> 8) & 0xff;
    UINT8 offset = (UINT8)(Address & 0xff); // TODO: Support extended addresses

    if (Width < EfiPciIoWidthUint8 || Width >= EfiPciIoWidthMaximum) {
        DEBUG((DEBUG_ERROR, "Invalid width %d\n", Width));
        return EFI_INVALID_PARAMETER;
    }

    width_bytes = Width == EfiPciIoWidthUint8 ? 1 :
                     Width == EfiPciIoWidthUint16 ? 2 :
                     Width == EfiPciIoWidthUint32 ? 4 : 8;

    while (Count--) {
        if (mmcfg) {
            if (bus > mmcfg_maxbus) {
                DEBUG((DEBUG_ERROR, "%s, Bus number %u exceeds max %u\n", __func__. bus, mmcfg_maxbus));
                return EFI_INVALID_PARAMETER;
            }
            pciConfigAccessMMCFG(bus, dev, function, offset, Width, Buffer, false);
        } else {
            switch (Width) {
                case EfiPciIoWidthUint8:
                    *(uint8_t *)Buffer = pciConfigReadByte(bus, dev, function, offset);

                    break;
                case EfiPciIoWidthUint16:
                    *(uint16_t *)Buffer = pciConfigReadWord(bus, dev, function, offset);
                    break;
                case EfiPciIoWidthUint32:
                    *(uint32_t *)Buffer = pciConfigReadDWord(bus, dev, function, offset);
                    break;
                case EfiPciIoWidthUint64:
                    DEBUG((DEBUG_ERROR, "64-bit read not supported\n"));
                    return EFI_UNSUPPORTED;
                default:
                    DEBUG((DEBUG_ERROR, "Unsupported width %d\n", Width));
                    return EFI_INVALID_PARAMETER;
            }
            Buffer += width_bytes;
            Address += width_bytes;
        }
    }

    return EFI_SUCCESS;
}

EFI_STATUS PciConfigWrite(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH Width, UINT64 Address,
                          UINTN Count, VOID *Buffer)
{
    UINTN width_bytes;
    UINT8 bus = (UINT8)(Address >> 24) & 0xff;
    UINT8 dev = (UINT8)(Address >> 16) & 0xff;
    UINT8 function = (UINT8)(Address >> 8) & 0xff;
    UINT8 offset = (UINT8)(Address & 0xff); // TODO: Support extended addresses

    if (Width < EfiPciIoWidthUint8 || Width >= EfiPciIoWidthMaximum) {
        DEBUG((DEBUG_ERROR, "Invalid width %d\n", Width));
        return EFI_INVALID_PARAMETER;
    }

    width_bytes = Width == EfiPciIoWidthUint8 ? 1 :
                     Width == EfiPciIoWidthUint16 ? 2 :
                     Width == EfiPciIoWidthUint32 ? 4 : 8;

    while (Count--) {
        if (mmcfg) {
            if (bus > mmcfg_maxbus) {
                DEBUG((DEBUG_ERROR, "%s, Bus number %u exceeds max %u\n", __func__. bus, mmcfg_maxbus));
                return EFI_INVALID_PARAMETER;
            }
            pciConfigAccessMMCFG(bus, dev, function, offset, Width, Buffer, true);
        } else {
            switch (Width) {
                case EfiPciIoWidthUint8:
                    pciConfigWriteByte(bus, dev, function, offset, *(uint8_t *)Buffer);
                    break;
                case EfiPciIoWidthUint16:
                    pciConfigWriteWord(bus, dev, function, offset, *(uint16_t *)Buffer);
                    break;
                case EfiPciIoWidthUint32:
                    pciConfigWriteDWord(bus, dev, function, offset, *(uint32_t *)Buffer);
                    break;
                case EfiPciIoWidthUint64:
                    DEBUG((DEBUG_ERROR, "64-bit write not supported\n"));
                    return EFI_UNSUPPORTED;
                default:
                    DEBUG((DEBUG_ERROR, "Unsupported width %d\n", Width));
                    return EFI_INVALID_PARAMETER;
            }
            Buffer += width_bytes;
            Address += width_bytes;
        }
    }

    return EFI_SUCCESS;
}

static EFI_STATUS pci_rootbus_probe(struct csmwrap_priv *priv)
{
    EFI_GUID EfiPciRootBridgeIoProtocolGuid = EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GUID;
    EFI_STATUS status;
    UINTN handle_count = 0;
    EFI_HANDLE *HandleBuffer;


    status = gBS->LocateHandleBuffer(ByProtocol, &EfiPciRootBridgeIoProtocolGuid,
                                    NULL, &handle_count, &HandleBuffer);

    if (EFI_ERROR(status)) {
        DEBUG((DEBUG_ERROR, "Failed to locate PCI Root Bridge IO Protocol handles: %d\n", status));
        goto out;
    }

    for (UINTN i = 0; i < handle_count; i++) {
        EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *pci_rb_io;
        void *resource = NULL;

        status = gBS->HandleProtocol(HandleBuffer[i], &EfiPciRootBridgeIoProtocolGuid,
                                     (VOID **)&pci_rb_io);
        if (EFI_ERROR(status)) {
            continue; // Skip handles that do not support the protocol
        }

        if (pci_rb_io->SegmentNumber != 0) {
            continue; // Only consider segment 0
        }

        // Check bus resources
        status = pci_rb_io->Configuration(pci_rb_io, (VOID **)&resource);
        if (EFI_ERROR(status)) {
            continue; // Skip if configuration cannot be retrieved
        }

        while (1) {
            ACPI_LARGE_RESOURCE_HEADER *res_header = (ACPI_LARGE_RESOURCE_HEADER *)resource;
            if (res_header->Header.Byte == ACPI_END_TAG_DESCRIPTOR) {
                break; // End of resources
            }

            if (res_header->Header.Byte == ACPI_QWORD_ADDRESS_SPACE_DESCRIPTOR) {
                EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *desc = (EFI_ACPI_ADDRESS_SPACE_DESCRIPTOR *)res_header;

                if (desc->ResType == ACPI_ADDRESS_SPACE_TYPE_BUS) {
                    // TODO: Will a single root bridge have multiple buses?
                    priv->rootbus_list[priv->rootbus_count] = desc->AddrRangeMin;
                    priv->rootbus_count++;
                    break;
                }
            }
            resource += res_header->Length + sizeof(ACPI_LARGE_RESOURCE_HEADER);
        }
    }

out:
    if (HandleBuffer) {
        gBS->FreePool(HandleBuffer);
    }

    if (priv->rootbus_count == 0) {
        DEBUG((DEBUG_ERROR, "No PCI Root Bridge IO Protocol found, fallback now\n"));
        priv->rootbus_count = 1;
        priv->rootbus_list[0] = 0; // Default to bus 0
        return EFI_NOT_FOUND;
    }

    for (int i = 0; i < priv->rootbus_count; i++) {
        DEBUG((DEBUG_INFO, "PCI Root Bridge at bus %u\n", priv->rootbus_list[i]));
    }

    return EFI_SUCCESS;
}

EFI_STATUS pci_init(struct csmwrap_priv *priv)
{
    EFI_STATUS Status;
    uacpi_status st;
    uacpi_table tbl;

    memset(&priv->hbridge_hdr, 0xf, sizeof(priv->hbridge_hdr));

    // Must after ACPI initialisation
    st = uacpi_table_find_by_signature(ACPI_MCFG_SIGNATURE, &tbl);
    while (st == UACPI_STATUS_OK) {
        UINTN i = 0;

        uacpi_table_ref(&tbl);

        struct acpi_mcfg *mcfg = (struct acpi_mcfg *)tbl.ptr;

        while (offsetof(struct acpi_mcfg, entries[i]) < tbl.hdr->length) {
            struct acpi_mcfg_allocation *alloc = &mcfg->entries[i];

            if (alloc->segment == 0) {
                if (mmcfg != NULL) {
                    DEBUG((DEBUG_ERROR, "Multiple MCFG entries found, using first one\n"));
                } else {
                    mmcfg = (void *)(uintptr_t)alloc->address;
                    mmcfg_maxbus = alloc->end_bus;

                    DEBUG((DEBUG_INFO, "MMCFG found at %p, max bus %u\n", mmcfg, mmcfg_maxbus));
                }
                break;
            }
            i++;
        }

        uacpi_table_unref(&tbl);
        st = uacpi_table_find_next_with_same_signature(&tbl);
    }

    Status = PciConfigRead(EfiPciIoWidthUint32, EFI_PCI_ADDRESS(0, 0, 0),
                           sizeof(priv->hbridge_hdr) / 4, &priv->hbridge_hdr);

    if (Status != EFI_SUCCESS || priv->hbridge_hdr.VendorId == 0xffff) {
        DEBUG((DEBUG_ERROR, "PCI configuration space not accessible\n"));
        return EFI_UNSUPPORTED;
    }

    printf("PCI initialisation: %s Host Bridge %04x:%04x\n",
           mmcfg ? "MMCFG" : "IO Port",
           priv->hbridge_hdr.VendorId, priv->hbridge_hdr.DeviceId);

    pci_rootbus_probe(priv);

    return EFI_SUCCESS;
}
