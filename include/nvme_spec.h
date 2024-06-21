#pragma once
#include <stddef.h>

namespace MiniNVMe
{

// https://www.nvmexpress.org/wp-content/uploads/NVM-Express-1_1.pdf

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

enum NVMeShnValue
{
    NVME_SHN_NORMAL = 0x1,
    NVME_SHN_ABRUPT = 0x2,
};

enum NVMeShstValue
{
    NVME_SHST_NORMAL = 0x0,
    NVME_SHST_SHUTTING = 0x1,
    NVME_SHST_COMPLETE = 0x2
};

struct NVMeCommand
{
    u8 opcode;
    u8 flags;
    u16 cid;
    u32 nsid;
    u64 reserved;
    u64 md_ptr;
    u64 dptr1;
    u64 dptr2;
    u32 cdw10;
    u32 cdw11;
    u32 cdw12;
    u32 cdw13;
    u32 cdw14;
    u32 cdw15;
};

struct NVMeCompletion
{
    u32 command_spec;
    u32 reserved;
    u16 sq_id;
    u16 sq_head;
    u16 cid;
    union
    {
        u16 status;
        struct
        {
            u16 phase : 1;
            u16 sc : 8;
            u16 sct : 3;
            u16 reserved2 : 2;
            u16 m : 1;
            u16 dnr : 1;
        };
     };
};

// – Controller Capabilities
union CapRegister
{
    u64 raw;
    struct
    {
        // Maximum Queue Entries Supported (MQES)
        u16 mqes;
        // Contiguous Queues Required (CQR)
        u16 cqr : 1;
        // Arbitration Mechanism Supported 
        u16 ams : 2;
        u16 reserved : 5;
        // Timeout
        u16 to : 8;
        // Doorbell Stride (DSTRD)
        u16 dstrd : 4;
        // NVM Subsystem Reset Supported (NSSRS)
        u16 nssrs : 1;
        // Command Sets Supported (CSS)
        u16 css : 8;
        u16 reserved1 : 3;
        // Memory Page Size Minimum (MPSMIN)
        u16 mpsmin : 4;
        // Memory Page Size Maximum (MPSMAX)
        u16 mpsmax : 4;
        u16 reserved2 : 8;
    } bits;
};
static_assert(sizeof(CapRegister) == 8, "invalid CapRegister size");

union VSRegister
{
    u32 raw;
    struct
    {
        u32 ter : 8;
        u32 mnr : 8;
        u32 mjr : 16;
    } bits;
};
static_assert(sizeof(VSRegister) == 4, "invalid VSRegister size");

union CtrlConfigurationRegister
{
    u32 raw;
    struct
    {
        u16 en : 1;
        u16 reserved : 3;
        u16 css : 3;
        u16 mps : 4;
        u16 ams : 3;
        u16 shn : 2;
        u16 iosqes : 4;
        u16 iocqes : 4;
        u16 reserved1 : 8;
    } bits;
};
static_assert(sizeof(CtrlConfigurationRegister) == 4, "invliad CtrlConfigurationRegister size");

union CSTSRegister
{
    u32 raw;
    struct
    {
        u16 rdy : 1;
        u16 cfs : 1;
        u16 shst : 2;
        u16 nssro : 1;
        u16 pp : 1;
        u32 reserved : 26;
    } bits;
};
static_assert(sizeof(CSTSRegister) == 4, "invalid CSTSRegister size");

union AQARegister
{
    u32 raw;
    struct
    {
        u16 asqs : 12;
        u16 reserved : 4;
        u16 acqs : 12;
        u16 reserved1 : 4;
    } bits;
};
static_assert(sizeof(AQARegister) == 4, "invalid AQARegister size");

union ASQRegister
{
    u32 raw;
    struct
    {
        u16 reserved : 12;
        u64 asqb : 52;
    } bits;
};
static_assert(sizeof(ASQRegister) == 8, "invalid ASQRegister size");

union ACQRegister
{
    u32 raw;
    struct
    {
        u16 reserved : 12;
        u64 acqb : 52;
    } bits;
};
static_assert(sizeof(ACQRegister) == 8, "invalid ACQRegister size");

static_assert(sizeof(CapRegister) + sizeof(VSRegister) + sizeof(u32) + sizeof(u32) + sizeof(CtrlConfigurationRegister) +
    sizeof(u32) + sizeof(CSTSRegister) + sizeof(u32) + sizeof(AQARegister) + sizeof(u64) + sizeof(u64) + sizeof(u32) * 4 +
    sizeof(u64) == 80, "invalid register size");
    
struct NVMeRegister
{
    CapRegister cap;               // 8B 0
    VSRegister  vs;                // 4B 8
    u32         intms;             // 4B 12
    u32         intmc;             // 4B 16
    CtrlConfigurationRegister cc;  // 4B 20
    u32         reserved;          // 4B 24
    CSTSRegister csts;             // 4B 28
    u32         nssr;
    AQARegister aqa;
    u64         asq;
    u64         acq;
    u32         cmbloc;
    u32         cmbsz;
    u32         bpinfo;
    u32         bprsel;
    u64         bpmbl; 
    u32         reserved1[0x3ec];
    u32         doorbell_base[1024];
};

static_assert(offsetof(NVMeRegister, doorbell_base) == 0x1000, "invalid doorbell base offset");

}
