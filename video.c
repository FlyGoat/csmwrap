#include <uefi.h>
#include "csmwrap.h"
#include "io.h"

int find_pci_vga(struct csmwrap_priv *priv)
{
     uint16_t bus;
     uint8_t device;
 
     for (bus = 0; bus < 256; bus++) {
         for (device = 0; device < 32; device++) {
            uint8_t function = 0;
            uint16_t vid;
            int maxfunc = 1;

            // It's a multi-function device, so check remaining functions
            if ((pciConfigReadByte(bus, device, function, 0xe) & 0x80) != 0)
                maxfunc = 8;
            for (function = 0; function < maxfunc; function++) {
                if (pciConfigReadWord(bus, device, function, 0x0) != 0xFFFF) {
                    printf("Scanning PCI bus %x:%x:%x\n", bus, device, function);
                    if (pciConfigReadByte(bus, device, function, 0xb) == 0x3) {
                        /* Do we need to disable ROM BAR? */
                        printf("Found VGA PCI %x:%x:%x, VID: %x DID: %x\n", bus, device, function,
                                    pciConfigReadWord(bus, device, function, 0x0),
                                    pciConfigReadWord(bus, device, function, 0x2));
                        priv->vga_pci_bus = bus;
                        priv->vga_devfn = device << 3 | function;
                        return 0;
                    }
                }
            }
         }
     }
    printf("PCI VGA not found!\n");
    return -1;
}

int csmwrap_video_init(struct csmwrap_priv *priv)
{
    struct csm_vga_table *vgabios = priv->vga_table;
    efi_status_t status;
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t *gop = NULL;
    efi_gop_mode_info_t *info = NULL;
    uintn_t isiz = sizeof(efi_gop_mode_info_t), currentMode, i;
    uintn_t target;

    status = BS->LocateProtocol(&gopGuid, NULL, (void**)&gop);

    find_pci_vga(priv);
    
    if(EFI_ERROR(status) && !gop) {
        printf("Unable to get GOP service\n");
        return -1;
    }

    /* FIXME: What if it's not a VBE mode? */
    currentMode = gop->Mode ? gop->Mode->Mode : 0;

    /* we got the interface, get current mode */
    status = gop->QueryMode(gop, currentMode, &isiz, &info);
    if(EFI_ERROR(status)) {
        printf("unable to get current video mode\n");
        return -1;
    }

    printf("%c %3d. %4d x%4d (pitch %4d fmt %d r:%06x g:%06x b:%06x)\n",
        '*', currentMode,
        info->HorizontalResolution, info->VerticalResolution, info->PixelsPerScanLine, info->PixelFormat,
        info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor?0xff:(
        info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff0000:(
        info->PixelFormat==PixelBitMask?info->PixelInformation.RedMask:0)),
        info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor ||
        info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff00:(
        info->PixelFormat==PixelBitMask?info->PixelInformation.GreenMask:0),
        info->PixelFormat==PixelRedGreenBlueReserved8BitPerColor?0xff0000:(
        info->PixelFormat==PixelBlueGreenRedReserved8BitPerColor?0xff:(
        info->PixelFormat==PixelBitMask?info->PixelInformation.BlueMask:0)));

    printf("EFI Framebuffer: %x\n", gop->Mode->FrameBufferBase);

    vgabios->physical_address = gop->Mode->FrameBufferBase;

    if (gop->Mode->FrameBufferBase >= 0xffffffff && priv->vga_pci_bus == 0 && priv->vga_devfn == (2 << 3) &&
        pciConfigReadWord(0, 2, 0, 0x0) == 0x8086) {
        printf("Attempt to Enable IGD Legacy Mapping for Intel \n");
        uint8_t val, orig;
        uint32_t val32, orig32;

        // MSR
        orig = inb(0x3cc); /* MSR Read 3CC */
        val |= (1 << 1); /* Set Memory Decoding Bit */
        outb(0x3c2, val); /* MSR Write 3C2 */
        val = inb(0x3cc); /* MSR Read 3CC */
        printf("IGD MSR: %x -> %x\n", orig, val);

        // GR06
        outb(0x3ce, 0x06); /* GR06 Write */
        orig = inb(0x3cf); /* GR06 Read */
        val = orig & ~((0x3) << 2); /* Clear Bit 2 and 3 */
        outb(0x3ce, 0x06); /* GR06 Write */
        outb(0x3cf, val); /* GR06 Write */
        val = inb(0x3cf); /* GR06 Read */
        printf("IGD GR06: %x -> %x\n", orig, val);

        // MGGC
        orig32 = pciConfigReadDWord(0, 2, 0, 0x50);
        val32 = orig32 & ~(1 << 1); /* Clear Bit 1 */
        pciConfigWriteDWord(0, 2, 0, 0x50, val32);
        val32 = pciConfigReadDWord(0, 2, 0, 0x50);
        printf("IGD MGGC: %x -> %x\n", val32, val32);

        vgabios->physical_address = 0xA0000;
    } else if (gop->Mode->FrameBufferBase > 0xffffffff) {
        printf("Framebuffer is too high, try Disabling Above 4G \n");
        return -1;
    }

    if (!vgabios->physical_address) {
        printf("Framebuffer invalid. try disable Above 4G\n");
        return -1;
    } else {
        printf("Actual framebuffer: %x\n", vgabios->physical_address);
    }
    /* FIXME: Assumed 32bbp, not good */
#if 1
    vgabios->bbp = 32;
    vgabios->x_resolution = info->HorizontalResolution;
    vgabios->y_resolution = info->VerticalResolution;
    vgabios->bytes_per_line = info->PixelsPerScanLine * (vgabios->bbp / 8);
#else
    vgabios->physical_address = gop->Mode->FrameBufferBase;
    vgabios->bbp = 32;
    vgabios->x_resolution = 800;
    vgabios->y_resolution = 600;
    vgabios->bytes_per_line = 800   * (vgabios->bbp / 8);
#endif


    /* Not going to reset mode */
# if 0
    status = gop->SetMode(gop, atoi(argv[1]));
    /* changing the resolution might mess up ConOut and StdErr, better to reset them */
    ST->ConOut->Reset(ST->ConOut, 0);
    ST->StdErr->Reset(ST->StdErr, 0);
    if(EFI_ERROR(status)) {
        printf("unable to set video mode\n");
        return 0;
    }
#endif

    return 0;
}
