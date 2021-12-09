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

            if (pciConfigReadWord(bus, device, function, 0x0) == 0xFFFF)
                break;
            // It's a multi-function device, so check remaining functions
            if ((pciConfigReadByte(bus, device, function, 0xe) & 0x80) != 0)
                maxfunc = 8;
            for (function = 0; function < maxfunc; function++) {
                if (pciConfigReadWord(bus, device, function, 0x0) != 0xFFFF) {
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
    currentMode = gop->Mode->Mode;

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

    /* FIXME: Assumed 32bbp, not good */
    vgabios->physical_address = gop->Mode->FrameBufferBase;
    vgabios->bbp = 32;
    vgabios->x_resolution = info->HorizontalResolution;
    vgabios->y_resolution = info->VerticalResolution;
    vgabios->bytes_per_line = info->PixelsPerScanLine * (vgabios->bbp / 8);


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
