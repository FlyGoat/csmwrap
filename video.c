#include <uefi.h>
#include "csmwrap.h"
#include "io.h"

efi_status_t FindGopPciDevice(struct csmwrap_priv *priv)
{
    EFI_STATUS                   Status = EFI_SUCCESS;
    EFI_HANDLE                   *HandleBuffer;
    EFI_HANDLE                   Handle;
    UINTN                        HandleCount;
    UINTN                        HandleIndex;
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_guid_t DevicePathGuid = EFI_DEVICE_PATH_PROTOCOL_GUID;
    efi_guid_t PciIoGuid = EFI_PCI_IO_PROTOCOL_GUID;
    EFI_DEVICE_PATH_PROTOCOL *DevicePath;
    EFI_PCI_IO_PROTOCOL *PciIo;
    efi_gop_t *Gop;

    // Get all handles that support GOP
    Status = gBS->LocateHandleBuffer(
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
        Status = gBS->HandleProtocol(
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
        gBS->FreePool(HandleBuffer);
        goto Out;
    }

    Status = gBS->HandleProtocol(
                    HandleBuffer[HandleIndex],
                    &DevicePathGuid,
                    (VOID**)&DevicePath
                    );
    // We are done with previous handle buffer atm
    gBS->FreePool(HandleBuffer);
    if (EFI_ERROR(Status)) {
        printf("Failed to get Device Path protocol: %d\n", Status);
        goto Out;
    }

    Status = gBS->LocateDevicePath(
        &PciIoGuid,
        &DevicePath,
        &Handle
        );

    if (EFI_ERROR(Status)) {
        printf("Failed to locate PCI I/O protocol: %d\n", Status);
        goto Out;
    }

    Status = gBS->HandleProtocol(
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
    efi_status_t Status;
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
    struct csm_vga_table *vgabios = priv->vga_table;
    efi_status_t status;
    efi_guid_t gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    efi_gop_t *gop = NULL;
    efi_gop_mode_info_t *info = NULL;
    uintn_t isiz = sizeof(efi_gop_mode_info_t), currentMode, i;
    uintn_t target;

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

    vgabios->physical_address = gop->Mode->FrameBufferBase;

    if (gop->Mode->FrameBufferBase > 0xffffffff) {
        printf("Framebuffer is too high, try Disabling Above 4G \n");
        return -1;
    }

    if (!vgabios->physical_address) {
        printf("Framebuffer invalid. try disable Above 4G\n");
        return -1;
    }

    /* FIXME: Assumed 32bbp, not good */
    vgabios->bbp = 32;
    vgabios->x_resolution = info->HorizontalResolution;
    vgabios->y_resolution = info->VerticalResolution;
    vgabios->bytes_per_line = info->PixelsPerScanLine * (vgabios->bbp / 8);

    return 0;
}

int csmwrap_video_fallback(struct csmwrap_priv *priv)
{
    struct csm_vga_table *vgabios = priv->vga_table;

    vgabios->physical_address = 0xa0000;
    vgabios->bbp = 32;
    vgabios->x_resolution = 800;
    vgabios->y_resolution = 600;
    vgabios->bytes_per_line = 800 * (vgabios->bbp / 8);

    printf("WARNING: Using fallback Video, you wont't be able to get display!\n");

    return 0;
}


