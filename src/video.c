#include <efi.h>
#include "csmwrap.h"
#include "io.h"

EFI_STATUS FindGopPciDevice(struct csmwrap_priv *priv)
{
    EFI_STATUS                   Status = EFI_SUCCESS;
    EFI_HANDLE                   *HandleBuffer;
    EFI_HANDLE                   Handle;
    UINTN                        HandleCount;
    UINTN                        HandleIndex;
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GUID DevicePathGuid = EFI_DEVICE_PATH_PROTOCOL_GUID;
    EFI_GUID PciIoGuid = EFI_PCI_IO_PROTOCOL_GUID;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    EFI_PCI_IO_PROTOCOL *PciIo;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *Gop;

    // Get all handles that support GOP
    Status = BS->LocateHandleBuffer(
                    ByProtocol,
                    &gopGuid,
                    NULL,
                    &HandleCount,
                    &HandleBuffer
                    );
    if (EFI_ERROR(Status)) {
        printf("Failed to locate GOP handles: %d\n", Status);
        return Status;
    }

    // Iterate through each GOP handle
    for (HandleIndex = 0; HandleIndex < HandleCount; HandleIndex++) {
        // Get the GOP protocol
        Status = BS->HandleProtocol(
                        HandleBuffer[HandleIndex],
                        &gopGuid,
                        (VOID**)&Gop
                        );
        if (EFI_ERROR(Status)) {
            continue;
        }

        priv->gop = Gop;
        break;
    }

    if (priv->gop == NULL) {
        printf("No GOP handle found\n");
        // Free the handle buffer
        BS->FreePool(HandleBuffer);
        goto Out;
    }

    Status = BS->HandleProtocol(
                    HandleBuffer[HandleIndex],
                    &DevicePathGuid,
                    (VOID**)&DevicePath
                    );
    // We are done with previous handle buffer atm
    BS->FreePool(HandleBuffer);
    if (EFI_ERROR(Status)) {
        printf("Failed to get Device Path protocol: %d\n", Status);
        goto Out;
    }

    Status = BS->LocateDevicePath(
        &PciIoGuid,
        &DevicePath,
        &Handle
        );

    if (EFI_ERROR(Status)) {
        printf("Failed to locate PCI I/O protocol: %d\n", Status);
        goto Out;
    }

    Status = BS->HandleProtocol(
                    Handle,
                    &PciIoGuid,
                    (VOID**)&PciIo
                    );

    if (!EFI_ERROR(Status)) {
        UINT16 VendorId, DeviceId;
        UINTN Seg, Bus, Device, Function;

        priv->vga_pci_io = PciIo;

        Status = PciIo->GetLocation(
                                    PciIo,
                                    &Seg,
                                    &Bus,
                                    &Device,
                                    &Function
                                    );

        priv->vga_pci_bus = (UINT8)Bus;
        priv->vga_pci_devfn = (UINT8)(Device << 3 | Function);

        Status = PciIo->Pci.Read(
                                PciIo,
                                EfiPciIoWidthUint16,
                                0, // Vendor ID offset
                                1,
                                &VendorId
                                );

        Status = PciIo->Pci.Read(
                                PciIo,
                                EfiPciIoWidthUint16,
                                2, // Device ID offset
                                1,
                                &DeviceId
                                );


        printf("GOP PCI: %04x:%02x:%02x.%02x %04x:%04x\n",
                    Seg, (UINT8)Bus, (UINT8)Device, (UINT8)Function,
                    VendorId, DeviceId);
    } else {
        printf("Failed to get PCI I/O protocol: %d\n", Status);
    }
Out:
  return Status;
}

static int csmwrap_pci_vgaarb(struct csmwrap_priv *priv)
{
    EFI_STATUS Status;
    EFI_PCI_IO_PROTOCOL *PciIo = priv->vga_pci_io;
    UINT64 Attributes = 0;
    UINT64 Supported = 0;
    BOOLEAN unsupported = FALSE;

    if (!PciIo) {
        return -1;
    }

    Status = PciIo->Attributes(PciIo, EfiPciIoAttributeOperationSupported,
                               0, &Supported);

    if (EFI_ERROR(Status)) {
        printf("%s: Failed to get supported attributes: %d\n", __func__, Status);
        return Status;
    }

    Attributes = Supported & (EFI_PCI_IO_ATTRIBUTE_VGA_IO | EFI_PCI_IO_ATTRIBUTE_VGA_IO_16);

    if (Attributes == 0) {
        printf("%s: No VGA IO attributes support\n", __func__);
        unsupported = TRUE;
    } else if (Attributes == (EFI_PCI_IO_ATTRIBUTE_VGA_IO | EFI_PCI_IO_ATTRIBUTE_VGA_IO_16)) {
        Attributes = EFI_PCI_IO_ATTRIBUTE_VGA_IO; // We want to use regular VGA IO
    }

    if (Supported & EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY) {
        Attributes |= EFI_PCI_IO_ATTRIBUTE_VGA_MEMORY;
    } else {
        printf("%s: No VGA memory attributes support\n", __func__);
        unsupported = TRUE;
    }

    if (unsupported) {
        printf("%s: Unable to select attribute\n", __func__);
        return -1;
    }

    Status = PciIo->Attributes(PciIo, EfiPciIoAttributeOperationEnable,
                               Attributes, NULL);
    if (EFI_ERROR(Status)) {
        printf("%s: Failed to set attributes: %d\n", __func__, Status);
        return Status;
    }

    printf("%s: Success! Attributes: %llx\n", __func__, Attributes);

    return 0;
}

int csmwrap_video_init(struct csmwrap_priv *priv)
{
    struct cb_framebuffer *cb_fb = &priv->cb_fb;
    EFI_STATUS status;
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop = NULL;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *info = NULL;
    UINTN isiz = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION), currentMode;

    status = FindGopPciDevice(priv);
    gop = priv->gop;

    if (EFI_ERROR(status) && !gop) {
        printf("Unable to get GOP service\n");
        return -1;
    }

    if (priv->vga_pci_io) {
        status = csmwrap_pci_vgaarb(priv);
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

    cb_fb->physical_address = gop->Mode->FrameBufferBase;

    if (!cb_fb->physical_address) {
        printf("Framebuffer invalid.\n");
        return -1;
    }

    if (gop->Mode->FrameBufferBase > 0xffffffff) {
        printf("Framebuffer is too high, try Disabling Above 4G \n");
        return -1;
    }

    cb_fb->x_resolution = info->HorizontalResolution;
    cb_fb->y_resolution = info->VerticalResolution;
    /* Always 32 bbp */
    cb_fb->bytes_per_line = info->PixelsPerScanLine * 4;
    cb_fb->bits_per_pixel = 32;

    switch (info->PixelFormat) {
        case PixelRedGreenBlueReserved8BitPerColor:
            cb_fb->red_mask_pos = 0;
            cb_fb->red_mask_size = 8;
            cb_fb->green_mask_pos = 8;
            cb_fb->green_mask_size = 8;
            cb_fb->blue_mask_pos = 16;
            cb_fb->blue_mask_size = 8;
            cb_fb->reserved_mask_pos = 24;
            cb_fb->reserved_mask_size = 8;
            break;
        case PixelBlueGreenRedReserved8BitPerColor:
            cb_fb->blue_mask_pos = 0;
            cb_fb->blue_mask_size = 8;
            cb_fb->green_mask_pos = 8;
            cb_fb->green_mask_size = 8;
            cb_fb->red_mask_pos = 16;
            cb_fb->red_mask_size = 8;
            cb_fb->reserved_mask_pos = 24;
            cb_fb->reserved_mask_size = 8;
            break;
        case PixelBitMask:
            // Calculate position (find first set bit)
            cb_fb->red_mask_pos = __builtin_ffs(info->PixelInformation.RedMask) - 1;
            cb_fb->green_mask_pos = __builtin_ffs(info->PixelInformation.GreenMask) - 1;
            cb_fb->blue_mask_pos = __builtin_ffs(info->PixelInformation.BlueMask) - 1;
            cb_fb->reserved_mask_pos = __builtin_ffs(info->PixelInformation.ReservedMask) - 1;

            // Calculate size (count set bits)
            cb_fb->red_mask_size = __builtin_popcount(info->PixelInformation.RedMask);
            cb_fb->green_mask_size = __builtin_popcount(info->PixelInformation.GreenMask);
            cb_fb->blue_mask_size = __builtin_popcount(info->PixelInformation.BlueMask);
            cb_fb->reserved_mask_size = __builtin_popcount(info->PixelInformation.ReservedMask);
            break;
        default:
            printf("Unsupported pixel format: %d\n", info->PixelFormat);
            return -1;
    }

    return 0;
}

int csmwrap_video_fallback(struct csmwrap_priv *priv)
{
    struct cb_framebuffer *cb_fb = &priv->cb_fb;

    cb_fb->physical_address = 0x000A0000;
    cb_fb->x_resolution = 1024;
    cb_fb->y_resolution = 768;
    cb_fb->bytes_per_line = 1024 * 4;
    cb_fb->bits_per_pixel = 32;
    cb_fb->red_mask_pos = 0;
    cb_fb->red_mask_size = 8;
    cb_fb->green_mask_pos = 8;
    cb_fb->green_mask_size = 8;
    cb_fb->blue_mask_pos = 16;
    cb_fb->blue_mask_size = 8;
    cb_fb->reserved_mask_pos = 24;
    cb_fb->reserved_mask_size = 8;

    printf("WARNING: Using fallback Video, you wont't be able to get display!\n");

    return 0;
}


