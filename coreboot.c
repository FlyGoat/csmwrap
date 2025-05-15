#include <uefi.h>
#include "csmwrap.h"

static UINT16
CbCheckSum16 (
  IN UINT16  *Buffer,
  IN UINTN   Length
  )
{
  UINT32  Sum;
  UINT32  TmpValue;
  UINTN   Idx;
  UINT8   *TmpPtr;

  Sum    = 0;
  TmpPtr = (UINT8 *)Buffer;
  for (Idx = 0; Idx < Length; Idx++) {
    TmpValue = TmpPtr[Idx];
    if (Idx % 2 == 1) {
      TmpValue <<= 8;
    }

    Sum += TmpValue;

    // Wrap
    if (Sum >= 0x10000) {
      Sum = (Sum + (Sum >> 16)) & 0xFFFF;
    }
  }

  return (UINT16)((~Sum) & 0xFFFF);
}

int build_coreboot_table(struct csmwrap_priv *priv)
{
        void *p = (void *)CB_TABLE_START;
        void *tables;
        uint32_t table_entries = 0;

        struct cb_header *header = (struct cb_header *)p;
        memset(header, 0, sizeof(struct cb_header));
        header->signature = CB_HEADER_SIGNATURE;
        header->header_bytes = sizeof(struct cb_header);
        p += header->header_bytes;
        tables = p;

        /* cb_framebuffer */
        struct cb_framebuffer *framebuffer = (struct cb_framebuffer *)p;
        memcpy(framebuffer, &priv->cb_fb, sizeof(struct cb_framebuffer));
        framebuffer->tag = CB_TAG_FRAMEBUFFER;
        framebuffer->size = sizeof(struct cb_framebuffer);
        p += framebuffer->size;
        table_entries++;

        /* Last header stuff */
        header->table_entries = table_entries;
        header->table_bytes = (uint32_t)((uintptr_t)p - (uintptr_t)tables);
        header->table_checksum = CbCheckSum16((UINT16*)tables, header->table_bytes);
        header->header_checksum = CbCheckSum16((UINT16*)header, header->header_bytes);

        return 0;
}
