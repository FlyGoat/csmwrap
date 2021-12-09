#include <uefi.h>

#include "csmwrap.h"
#include "io.h"

int unlock_bios_region()
{
    printf("Host Bridge ID: %x\n", pciConfigReadDWord(0, 0, 0, 0x0));
    if (pciConfigReadDWord(0, 0, 0, 0x0) == 0x29c08086) {
        printf("Unlocking BIOS region with Q35 PAM\n");
        pciConfigWriteByte(0, 0, 0, 0x90, 0x33);
        pciConfigWriteByte(0, 0, 0, 0x91, 0x33);
        pciConfigWriteByte(0, 0, 0, 0x92, 0x33);
        pciConfigWriteByte(0, 0, 0, 0x93, 0x33);
        pciConfigWriteByte(0, 0, 0, 0x94, 0x33);
        pciConfigWriteByte(0, 0, 0, 0x95, 0x33);
        pciConfigWriteByte(0, 0, 0, 0x96, 0x33);
        return 0;
    }

    if (pciConfigReadWord(0, 0, 0, 0x0) == 0x8086) {
        printf("Unlocking BIOS region with Intel Skylake+ Generic PAM\n");
        if (pciConfigReadByte(0, 0, 0, 0x80) & 0x1) {
            printf("PAM is locked on your platform\n");
            return -1;
        }

        pciConfigWriteByte(0, 0, 0, 0x80, 0x30);
        pciConfigWriteByte(0, 0, 0, 0x81, 0x33);
        pciConfigWriteByte(0, 0, 0, 0x82, 0x33);
        pciConfigWriteByte(0, 0, 0, 0x83, 0x33);
        pciConfigWriteByte(0, 0, 0, 0x84, 0x33);
        pciConfigWriteByte(0, 0, 0, 0x85, 0x33);
        pciConfigWriteByte(0, 0, 0, 0x86, 0x33);
        return 0;
    }

    return -1;
}
