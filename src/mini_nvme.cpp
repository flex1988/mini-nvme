#include "mini_nvme.h"
#include "memory.h"
#include <immintrin.h>
#include <unistd.h>
#include <stddef.h>
#include <string.h>

namespace MiniNVMe
{

void ShowCCRegs(CtrlConfigurationRegister* cc)
{
    printf("[Controller Configuration] en %d css %d mps %d iosqes %d iocqes %d shn 0x%x\n",
            cc->bits.en, cc->bits.css, cc->bits.mps,
            cc->bits.iosqes, cc->bits.iocqes, cc->bits.shn);
}

void ShowCSTSRegs(CSTSRegister* csts)
{
    printf("[Controller Status] shst %d rdy %d\n", csts->bits.shst, csts->bits.rdy);
}

NVMeDriver::NVMeDriver()
: _pci(new PCI)
{
}

NVMeDriver::~NVMeDriver()
{
    delete _pci;
}

bool NVMeDriver::Init(const std::string& pci_addr)
{
    if (!_pci->Init(pci_addr))
    {
        printf("mini nvme init pci failed\n");
        return false;
    }
    PCIResource res = _pci->MapResource("resource0");
    if (res.ptr == nullptr)
    {
        printf("mini nvme map pci resource failed\n");
        return false;
    }
    _nvme_regs = (NVMeRegister*)res.ptr;
    
    _cap.max_queue_entries = _nvme_regs->cap.bits.mqes;
    _cap.timeout = 500 * _nvme_regs->cap.bits.to;
    _cap.doorbell_stride = 1 << (2 + _nvme_regs->cap.bits.dstrd);
    _cap.memory_page_size_min = 1 << (12 + _nvme_regs->cap.bits.mpsmin);
    _cap.memory_page_size_max = 1 << (12 + _nvme_regs->cap.bits.mpsmax);
    printf("[Controller Capabilities] MQES %d Timeout %dms DSTRD %d bytes MPSMIN %d bytes MPSMAX %d bytes\n",
            _cap.max_queue_entries + 1, _cap.timeout, _cap.doorbell_stride, _cap.memory_page_size_min, _cap.memory_page_size_max);

    _nvme_major_version = _nvme_regs->vs.bits.mjr;
    _nvme_minor_version = _nvme_regs->vs.bits.mnr;
    printf("[NVMe specification] %u.%u\n", _nvme_major_version, _nvme_minor_version);

    ShowCCRegs(&_nvme_regs->cc);

    printf("[Ctrl Status] rdy 0x%x shst 0x%x\n", _nvme_regs->csts.bits.rdy, _nvme_regs->csts.bits.shst);

    AQARegister* aqa = (AQARegister*)&_nvme_regs->aqa;
    printf("[AQA] ACQS %d ASQS %d\n", aqa->bits.asqs, aqa->bits.acqs);
    
    ASQRegister* asq = (ASQRegister*)&_nvme_regs->asq;
    printf("[ASQ] ASQB %x\n", asq->bits.asqb);

    ACQRegister* acq = (ACQRegister*)&_nvme_regs->acq;
    printf("[ACQ] ACQB %x\n", acq->bits.acqb);

    MiniNVMe::DmaAllocator* allocator = new MiniNVMe::DmaAllocator("/mnt/huge", 1 << 30, 32 << 20);
    allocator->Init();

    CtrlConfigurationRegister cc;
    CSTSRegister csts;
    
    cc.raw = GetRegs<u32>(offsetof(NVMeRegister, cc));
    if (cc.bits.en)
    {
            printf("1. controller is enable wait for ready\n");
	    do
	    {
		sleep(1);
		csts.raw = GetRegs<u32>(offsetof(NVMeRegister, csts.raw));
                ShowCSTSRegs(&csts);
	    } while(csts.bits.rdy == 0);
            printf("2. controller is ready now\n");

	    printf("3. disable controller\n");
            cc.bits.en = 0;
            SetRegs<u32>(offsetof(NVMeRegister, cc.raw), cc.raw);
    }

    static_assert(offsetof(NVMeRegister, csts) == 28, "invalid csts register offset");
    // 2. wait csts.rdy == 0
    do
    {
        csts.raw = GetRegs<u32>(offsetof(NVMeRegister, csts));
        ShowCSTSRegs(&csts);
        sleep(1);
    } while (csts.bits.rdy == 1);

    // 3. set admin qpair
    _admin_sq = allocator->Alloc<NVMeSQ>();
    _admin_sq->commands = allocator->Alloc<NVMeCommand>(1024);
    _admin_sq->head = 0;
    _admin_sq->tail = 0;
    _admin_sq->len = 1024;
    _admin_sq->doorbell = 0;
    
    _admin_cq = allocator->Alloc<NVMeCQ>();
    _admin_cq->completions = allocator->Alloc<NVMeCompletion>(1024);
    _admin_cq->phase = 0;
    _admin_cq->head = 0;
    _admin_cq->len = 1024;

    // 4. set admin submission queue and admin completion queue
    SetRegs<u64>(offsetof(NVMeRegister, acq), allocator->VirtToPhys(_admin_cq->completions));
    SetRegs<u64>(offsetof(NVMeRegister, asq), allocator->VirtToPhys(_admin_sq->commands));
    SetRegs<u32>(offsetof(NVMeRegister, aqa), 1023u << 16 | 1023u);

    // 5. set cc.bits.en = 1, enable controller
    cc.raw = GetRegs<u32>(offsetof(NVMeRegister, cc.raw));
    cc.bits.en = 1;
    SetRegs<u32>(offsetof(struct NVMeRegister, cc.raw), cc.raw);

    ShowCCRegs(&cc);

    // 6. wait csts.rdy == 1
    while (_nvme_regs->csts.bits.rdy == 0)
    {
        printf("wait for controller started... csts.shst %d cc.shn %d\n",
                _nvme_regs->csts.bits.shst, _nvme_regs->cc.bits.shn);
        sleep(1);
    }

    volatile u32* doorbell_base = (volatile u32*)&_nvme_regs->doorbell_base[0];
    volatile u32* admin_sq_tdbl = doorbell_base;
    volatile u32* admin_cq_hdbl = doorbell_base + 1;

    void* buffer = allocator->Alloc<u64>(4096 / 8, 4096);

    // 7. identify controller 
    int cid = _admin_sq->tail;
    NVMeCommand* cmd = reinterpret_cast<NVMeCommand*>(_admin_sq->commands) + cid;
    memset(cmd, 0, sizeof(NVMeCommand));
    cmd->opcode = 6;
    // cid means sq_tail
    cmd->cid = cid;
    cmd->nsid = 0;
    cmd->dptr1 = allocator->VirtToPhys(buffer);
    cmd->dptr2 = cmd->dptr1 + 4096;
    cmd->cdw10 = 1;
    _mm_mfence();
    *(volatile u32*)admin_sq_tdbl = _admin_sq->tail;

    _admin_sq->tail++;
    // 8. doorbell submission queue tail doorbell
    NVMeCompletion* completion = reinterpret_cast<NVMeCompletion*>(_admin_cq->completions) + _admin_cq->head;

    while (completion->phase == _admin_cq->phase)
    {
        printf("wait identify msg %d %d\n", completion->phase, _admin_cq->phase);
        sleep(1);
    }

    _admin_cq->head++;
    if (_admin_cq->head == _admin_cq->len)
    {
        _admin_cq->head = 0;
        _admin_cq->phase = !_admin_cq->phase;
    }
   
    _mm_mfence();
    *(volatile u32*)admin_cq_hdbl = _admin_cq->head;

    printf("admin sq tail doorbell %u cq head doorbell %d\n", *(volatile u32*)admin_sq_tdbl, *(volatile u32*)admin_cq_hdbl);

    return true;
}

template <typename T>
void NVMeDriver::SetRegs(u32 offset, T value)
{
    _mm_mfence();
    *reinterpret_cast<volatile T*>((char*)_nvme_regs + offset) = value;
}

template <typename T>
T NVMeDriver::GetRegs(u32 offset)
{
    _mm_mfence();
    return *reinterpret_cast<volatile T*>((char*)_nvme_regs + offset);
}

bool NVMeDriver::Start()
{
    return false;
}

bool NVMeDriver::Stop()
{
    return false;
}

bool NVMeDriver::Deinit()
{
    return false;
}


}
