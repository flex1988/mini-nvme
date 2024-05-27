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

// – Controller Capabilities
struct CapRegister
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
};
static_assert(sizeof(CapRegister) == 8, "invalid CapRegister size");

struct VSRegister
{
    u32 ter : 8;
    u32 mnr : 8;
    u32 mjr : 16;
};
static_assert(sizeof(VSRegister) == 4, "invalid VSRegister size");

struct CtrlConfigurationRegister
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
};
static_assert(sizeof(CtrlConfigurationRegister) == 4, "invliad CtrlConfigurationRegister size");

struct CSTSRegister
{
    u16 rdy : 1;
    u16 cfs : 1;
    u16 shst : 2;
    u16 nssro : 1;
    u16 pp : 1;
    u32 reserved : 26;
};
static_assert(sizeof(CSTSRegister) == 4, "invalid CSTSRegister size");

struct AQARegister
{
    u16 asqs : 12;
    u16 reserved : 4;
    u16 acqs : 12;
    u16 reserved1 : 4;
};
static_assert(sizeof(AQARegister) == 4, "invalid AQARegister size");

struct ASQRegister
{
    u16 reserved : 12;
    u64 asqb : 52;
};
static_assert(sizeof(ASQRegister) == 8, "invalid ASQRegister size");

struct ACQRegister
{
    u16 reserved : 12;
    u64 acqb : 52;
};
static_assert(sizeof(ACQRegister) == 8, "invalid ACQRegister size");
    
struct NVMeRegister
{
    CapRegister cap;
    VSRegister  vs;
    u32         intms;
    u32         intmc;
    CtrlConfigurationRegister cc;
    u32         reserved;
    CSTSRegister csts;
    u32         nssr;
    AQARegister aqa;
    u64         asq;
    u64         acq;
};

}
