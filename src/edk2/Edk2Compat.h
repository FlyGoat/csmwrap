#ifndef _EDK2_COMPAT_H_
#define _EDK2_COMPAT_H_

#include <efi.h>

/* Packed is already handled by pragmas */
#define PACKED

#define GUID EFI_GUID

#define SIGNATURE_16(A,B)               EFI_SIGNATURE_16(A,B)
#define SIGNATURE_32(A,B,C,D)           EFI_SIGNATURE_32(A,B,C,D)
#define SIGNATURE_64(A,B,C,D,E,F,G,H)   EFI_SIGNATURE_64(A,B,C,D,E,F,G,H)

/* Debug Macros */

//
// Declare bits for PcdDebugPrintErrorLevel and the ErrorLevel parameter of DebugPrint()
//
#define DEBUG_INIT      0x00000001       // Initialization
#define DEBUG_WARN      0x00000002       // Warnings
#define DEBUG_LOAD      0x00000004       // Load events
#define DEBUG_FS        0x00000008       // EFI File system
#define DEBUG_POOL      0x00000010       // Alloc & Free (pool)
#define DEBUG_PAGE      0x00000020       // Alloc & Free (page)
#define DEBUG_INFO      0x00000040       // Informational debug messages
#define DEBUG_DISPATCH  0x00000080       // PEI/DXE/SMM Dispatchers
#define DEBUG_VARIABLE  0x00000100       // Variable
#define DEBUG_BM        0x00000400       // Boot Manager
#define DEBUG_BLKIO     0x00001000       // BlkIo Driver
#define DEBUG_NET       0x00004000       // Network Io Driver
#define DEBUG_UNDI      0x00010000       // UNDI Driver
#define DEBUG_LOADFILE  0x00020000       // LoadFile
#define DEBUG_EVENT     0x00080000       // Event messages
#define DEBUG_GCD       0x00100000       // Global Coherency Database changes
#define DEBUG_CACHE     0x00200000       // Memory range cachability changes
#define DEBUG_VERBOSE   0x00400000       // Detailed debug messages that may
                                         // significantly impact boot performance
#define DEBUG_MANAGEABILITY  0x00800000  // Detailed debug and payload manageability messages
                                         // related to modules such as Redfish, IPMI, MCTP etc.
#define DEBUG_ERROR  0x80000000          // Error messages

#ifndef DEBUG_PRINT_LEVEL
#define DEBUG_PRINT_LEVEL  (DEBUG_ERROR)
#endif

#define _DEBUG_PRINT(PrintLevel, ...)              \
    do {                                             \
      if (PrintLevel & DEBUG_PRINT_LEVEL) {     \
        printf (#__VA_ARGS__);      \
      }                                              \
    } while (FALSE)
#define _DEBUGLIB_DEBUG(Expression)  _DEBUG_PRINT Expression

#define DEBUG(Expression)        \
    do {                           \
      if (TRUE) {                 \
        _DEBUGLIB_DEBUG (Expression);       \
      }                            \
    } while (FALSE)

#endif /* _EDK2_COMPAT_H_ */
