/*
 * Real Mode Thunk Functions for IA32 and x64.
 *
 * Based on various EDK2 headers
 *
 * Which is:
 * Copyright (c) 2006 - 2018, Intel Corporation. All rights reserved.<BR>
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */

#ifndef _X86_THUNK_H
#define _X86_THUNK_H

#include <uefi.h>

#define EFI_SEGMENT(_Adr)  (uint16_t) ((uint16_t) (((uintptr_t) (_Adr)) >> 4) & 0xf000)
#define EFI_OFFSET(_Adr)   (uint16_t) (((uint16_t) ((uintptr_t) (_Adr))) & 0xffff)

#pragma pack (1)
///
/// Byte packed structure for EFLAGS/RFLAGS.
/// 32-bits on IA-32.
/// 64-bits on x64.  The upper 32-bits on x64 are reserved.
///
typedef union {
  struct {
    uint32_t    CF         : 1;  ///< Carry Flag.
    uint32_t    Reserved_0 : 1;  ///< Reserved.
    uint32_t    PF         : 1;  ///< Parity Flag.
    uint32_t    Reserved_1 : 1;  ///< Reserved.
    uint32_t    AF         : 1;  ///< Auxiliary Carry Flag.
    uint32_t    Reserved_2 : 1;  ///< Reserved.
    uint32_t    ZF         : 1;  ///< Zero Flag.
    uint32_t    SF         : 1;  ///< Sign Flag.
    uint32_t    TF         : 1;  ///< Trap Flag.
    uint32_t    IF         : 1;  ///< Interrupt Enable Flag.
    uint32_t    DF         : 1;  ///< Direction Flag.
    uint32_t    OF         : 1;  ///< Overflow Flag.
    uint32_t    IOPL       : 2;  ///< I/O Privilege Level.
    uint32_t    NT         : 1;  ///< Nested Task.
    uint32_t    Reserved_3 : 1;  ///< Reserved.
    uint32_t    RF         : 1;  ///< Resume Flag.
    uint32_t    VM         : 1;  ///< Virtual 8086 Mode.
    uint32_t    AC         : 1;  ///< Alignment Check.
    uint32_t    VIF        : 1;  ///< Virtual Interrupt Flag.
    uint32_t    VIP        : 1;  ///< Virtual Interrupt Pending.
    uint32_t    ID         : 1;  ///< ID Flag.
    uint32_t    Reserved_4 : 10; ///< Reserved.
  } Bits;
  uintptr_t    UintN;
} IA32_EFLAGS32;

///
/// Byte packed structure for an x64 Interrupt Gate Descriptor.
///
typedef union {
  struct {
    uint32_t    OffsetLow   : 16; ///< Offset bits 15..0.
    uint32_t    Selector    : 16; ///< Selector.
    uint32_t    Reserved_0  : 8;  ///< Reserved.
    uint32_t    GateType    : 8;  ///< Gate Type.  See #defines above.
    uint32_t    OffsetHigh  : 16; ///< Offset bits 31..16.
    uint32_t    OffsetUpper : 32; ///< Offset bits 63..32.
    uint32_t    Reserved_1  : 32; ///< Reserved.
  } Bits;
  struct {
    uint64_t    Uint64;
    uint64_t    Uint64_1;
  } Uint128;
} IA32_IDT_GATE_DESCRIPTOR;


//
// IA32 Task-State Segment Definition
//
typedef struct {
  uint32_t    Reserved_0;
  uint64_t    RSP0;
  uint64_t    RSP1;
  uint64_t    RSP2;
  uint64_t    Reserved_28;
  uint64_t    IST[7];
  uint64_t    Reserved_92;
  uint16_t    Reserved_100;
  uint16_t    IOMapBaseAddress;
} IA32_TASK_STATE_SEGMENT;

typedef union {
  struct {
    uint32_t    LimitLow    : 16; ///< Segment Limit 15..00
    uint32_t    BaseLow     : 16; ///< Base Address  15..00
    uint32_t    BaseMidl    : 8;  ///< Base Address  23..16
    uint32_t    Type        : 4;  ///< Type (1 0 B 1)
    uint32_t    Reserved_43 : 1;  ///< 0
    uint32_t    DPL         : 2;  ///< Descriptor Privilege Level
    uint32_t    P           : 1;  ///< Segment Present
    uint32_t    LimitHigh   : 4;  ///< Segment Limit 19..16
    uint32_t    AVL         : 1;  ///< Available for use by system software
    uint32_t    Reserved_52 : 2;  ///< 0 0
    uint32_t    G           : 1;  ///< Granularity
    uint32_t    BaseMidh    : 8;  ///< Base Address  31..24
    uint32_t    BaseHigh    : 32; ///< Base Address  63..32
    uint32_t    Reserved_96 : 32; ///< Reserved
  } Bits;
  struct {
    uint64_t    Uint64;
    uint64_t    Uint64_1;
  } Uint128;
} IA32_TSS_DESCRIPTOR;

///
/// Byte packed structure for an FP/SSE/SSE2 context.
///
typedef struct {
  uint8_t    Buffer[512];
} IA32_FX_BUFFER;

///
/// Structures for the 16-bit real mode thunks.
///
typedef struct {
  uint32_t    Reserved1;
  uint32_t    Reserved2;
  uint32_t    Reserved3;
  uint32_t    Reserved4;
  uint8_t     BL;
  uint8_t     BH;
  uint16_t    Reserved5;
  uint8_t     DL;
  uint8_t     DH;
  uint16_t    Reserved6;
  uint8_t     CL;
  uint8_t     CH;
  uint16_t    Reserved7;
  uint8_t     AL;
  uint8_t     AH;
  uint16_t    Reserved8;
} IA32_BYTE_REGS;

typedef struct {
  uint16_t    DI;
  uint16_t    Reserved1;
  uint16_t    SI;
  uint16_t    Reserved2;
  uint16_t    BP;
  uint16_t    Reserved3;
  uint16_t    SP;
  uint16_t    Reserved4;
  uint16_t    BX;
  uint16_t    Reserved5;
  uint16_t    DX;
  uint16_t    Reserved6;
  uint16_t    CX;
  uint16_t    Reserved7;
  uint16_t    AX;
  uint16_t    Reserved8;
} IA32_WORD_REGS;

typedef struct {
  uint32_t           EDI;
  uint32_t           ESI;
  uint32_t           EBP;
  uint32_t           ESP;
  uint32_t           EBX;
  uint32_t           EDX;
  uint32_t           ECX;
  uint32_t           EAX;
  uint16_t           DS;
  uint16_t           ES;
  uint16_t           FS;
  uint16_t           GS;
  IA32_EFLAGS32    EFLAGS;
  uint32_t           Eip;
  uint16_t           CS;
  uint16_t           SS;
} IA32_DWORD_REGS;

typedef union {
  IA32_DWORD_REGS    E;
  IA32_WORD_REGS     X;
  IA32_BYTE_REGS     H;
} IA32_REGISTER_SET;

typedef union {
  struct {
    uint32_t    LimitLow  : 16;
    uint32_t    BaseLow   : 16;
    uint32_t    BaseMid   : 8;
    uint32_t    Type      : 4;
    uint32_t    S         : 1;
    uint32_t    DPL       : 2;
    uint32_t    P         : 1;
    uint32_t    LimitHigh : 4;
    uint32_t    AVL       : 1;
    uint32_t    L         : 1;
    uint32_t    DB        : 1;
    uint32_t    G         : 1;
    uint32_t    BaseHigh  : 8;
  } Bits;
  uint64_t    Uint64;
} IA32_SEGMENT_DESCRIPTOR;

///
/// Byte packed structure for an 16-bit real mode thunks.
///
typedef struct {
  IA32_REGISTER_SET    *RealModeState;
  void                 *RealModeBuffer;
  uint32_t               RealModeBufferSize;
  uint32_t               ThunkAttributes;
} THUNK_CONTEXT;

#define THUNK_ATTRIBUTE_BIG_REAL_MODE              0x00000001
#define THUNK_ATTRIBUTE_DISABLE_A20_MASK_INT_15    0x00000002
#define THUNK_ATTRIBUTE_DISABLE_A20_MASK_KBD_CTRL  0x00000004

///
/// EFI_EFLAGS_REG
///
typedef struct {
  uint32_t    CF        : 1;
  uint32_t    Reserved1 : 1;
  uint32_t    PF        : 1;
  uint32_t    Reserved2 : 1;
  uint32_t    AF        : 1;
  uint32_t    Reserved3 : 1;
  uint32_t    ZF        : 1;
  uint32_t    SF        : 1;
  uint32_t    TF        : 1;
  uint32_t    IF        : 1;
  uint32_t    DF        : 1;
  uint32_t    OF        : 1;
  uint32_t    IOPL      : 2;
  uint32_t    NT        : 1;
  uint32_t    Reserved4 : 2;
  uint32_t    VM        : 1;
  uint32_t    Reserved5 : 14;
} EFI_EFLAGS_REG;

///
/// EFI_DWORD_REGS
///
typedef struct {
  uint32_t            EAX;
  uint32_t            EBX;
  uint32_t            ECX;
  uint32_t            EDX;
  uint32_t            ESI;
  uint32_t            EDI;
  EFI_EFLAGS_REG    EFlags;
  uint16_t            ES;
  uint16_t            CS;
  uint16_t            SS;
  uint16_t            DS;
  uint16_t            FS;
  uint16_t            GS;
  uint32_t            EBP;
  uint32_t            ESP;
} EFI_DWORD_REGS;

///
/// EFI_FLAGS_REG
///
typedef struct {
  uint16_t    CF        : 1;
  uint16_t    Reserved1 : 1;
  uint16_t    PF        : 1;
  uint16_t    Reserved2 : 1;
  uint16_t    AF        : 1;
  uint16_t    Reserved3 : 1;
  uint16_t    ZF        : 1;
  uint16_t    SF        : 1;
  uint16_t    TF        : 1;
  uint16_t    IF        : 1;
  uint16_t    DF        : 1;
  uint16_t    OF        : 1;
  uint16_t    IOPL      : 2;
  uint16_t    NT        : 1;
  uint16_t    Reserved4 : 1;
} EFI_FLAGS_REG;

///
/// EFI_WORD_REGS
///
typedef struct {
  uint16_t           AX;
  uint16_t           ReservedAX;
  uint16_t           BX;
  uint16_t           ReservedBX;
  uint16_t           CX;
  uint16_t           ReservedCX;
  uint16_t           DX;
  uint16_t           ReservedDX;
  uint16_t           SI;
  uint16_t           ReservedSI;
  uint16_t           DI;
  uint16_t           ReservedDI;
  EFI_FLAGS_REG    Flags;
  uint16_t           ReservedFlags;
  uint16_t           ES;
  uint16_t           CS;
  uint16_t           SS;
  uint16_t           DS;
  uint16_t           FS;
  uint16_t           GS;
  uint16_t           BP;
  uint16_t           ReservedBP;
  uint16_t           SP;
  uint16_t           ReservedSP;
} EFI_WORD_REGS;

///
/// EFI_BYTE_REGS
///
typedef struct {
  uint8_t     AL, AH;
  uint16_t    ReservedAX;
  uint8_t     BL, BH;
  uint16_t    ReservedBX;
  uint8_t     CL, CH;
  uint16_t    ReservedCX;
  uint8_t     DL, DH;
  uint16_t    ReservedDX;
} EFI_BYTE_REGS;

///
/// EFI_IA32_REGISTER_SET
///
typedef union {
  EFI_DWORD_REGS    E;
  EFI_WORD_REGS     X;
  EFI_BYTE_REGS     H;
} EFI_IA32_REGISTER_SET;

#pragma pack(1)

#define NUM_REAL_GDT_ENTRIES     8
#define CONVENTIONAL_MEMORY_TOP  0xA0000  // 640 KB
#define INITIAL_VALUE_BELOW_1K   0x0

//
// Define what a processor GDT looks like
//
typedef struct {
  uint16_t    LimitLow;
  uint16_t    BaseLow;
  uint8_t     BaseMid;
  uint8_t     Attribute;
  uint8_t     LimitHi;
  uint8_t     BaseHi;
} GDT64;

//
// Define what a processor descriptor looks like
// This data structure must be kept in sync with ASM STRUCT in Thunk.inc
//
typedef struct {
  uint16_t    Limit;
  uint64_t    Base;
} DESCRIPTOR64;

typedef struct {
  uint16_t    Limit;
  uint32_t    Base;
} DESCRIPTOR32;

//
// Low stub lay out
//
#define LOW_STACK_SIZE      (8 * 1024)  // 8k?
#define EFI_MAX_E820_ENTRY  100
#define FIRST_INSTANCE      1
#define NOT_FIRST_INSTANCE  0

typedef struct {
  //
  // Space for the code
  //  The address of Code is also the beginning of the relocated Thunk code
  //
  uint8_t                                Code[4096]; // ?

  //
  // Data for the code (cs releative)
  //
  DESCRIPTOR64                         X64GdtDesc;       // Protected mode GDT
  DESCRIPTOR64                         X64IdtDesc;       // Protected mode IDT
  uintptr_t                            X64Ss;
  uintptr_t                            X64Esp;

  uintptr_t                            RealStack;
  DESCRIPTOR32                         RealModeIdtDesc;
  DESCRIPTOR32                         RealModeGdtDesc;

  //
  // real-mode GDT (temporary GDT with two real mode segment descriptors)
  //
  GDT64                                RealModeGdt[NUM_REAL_GDT_ENTRIES];
  uint64_t                             PageMapLevel4;

  //
  // A low memory stack
  //
  uint8_t                              Stack[LOW_STACK_SIZE];
} LOW_MEMORY_THUNK;

#pragma pack()

extern uintptr_t LegacyBiosInitializeThunkAndTable(uintptr_t MemoryAddress, size_t data_size);

extern boolean_t LegacyBiosFarCall86 (uint16_t Segment, uint16_t Offset, EFI_IA32_REGISTER_SET *Regs, void *Stack, uintptr_t StackSize);

#endif
