#include <efi.h>
#include <csmwrap.h>

EFI_STATUS PciConfigRead(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH Width, UINT64 Address,
                         UINTN Count, VOID *Buffer);
EFI_STATUS PciConfigWrite(EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_WIDTH Width, UINT64 Address,
                          UINTN Count, VOID *Buffer);
EFI_STATUS pci_init(struct csmwrap_priv *priv);
