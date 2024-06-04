#include "mini_nvme.h"
#include "memory.h"
#include <immintrin.h>
#include <unistd.h>
#include <stddef.h>

namespace MiniNVMe
{

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

    printf("[Controller Configuration] en %d css %d mps %d iosqes %d iocqes %d shn 0x%x\n",
            _nvme_regs->cc.bits.en, _nvme_regs->cc.bits.css, _nvme_regs->cc.bits.mps,
            _nvme_regs->cc.bits.iosqes, _nvme_regs->cc.bits.iocqes, _nvme_regs->cc.bits.shn);

    printf("[Ctrl Status] rdy 0x%x shst 0x%x\n", _nvme_regs->csts.bits.rdy, _nvme_regs->csts.bits.shst);

    AQARegister* aqa = (AQARegister*)&_nvme_regs->aqa;
    printf("[AQA] ACQS %d ASQS %d\n", aqa->bits.asqs, aqa->bits.acqs);
    
    ASQRegister* asq = (ASQRegister*)&_nvme_regs->asq;
    printf("[ASQ] ASQB %x\n", asq->bits.asqb);

    ACQRegister* acq = (ACQRegister*)&_nvme_regs->acq;
    printf("[ACQ] ACQB %x\n", acq->bits.acqb);

    MiniNVMe::DmaAllocator* allocator = new MiniNVMe::DmaAllocator("/mnt/huge", 1 << 30, 32 << 20);
    allocator->Init();

    // 1. set cc.en = 1
    CtrlConfigurationRegister cc = _nvme_regs->cc;
    cc.bits.en = 0;
    SetRegs<u32>(offsetof(struct NVMeRegister, cc.raw), cc.raw);

    printf("[Controller Configuration] en %d css %d mps %d iosqes %d iocqes %d shn 0x%x\n",
            _nvme_regs->cc.bits.en, _nvme_regs->cc.bits.css, _nvme_regs->cc.bits.mps,
            _nvme_regs->cc.bits.iosqes, _nvme_regs->cc.bits.iocqes, _nvme_regs->cc.bits.shn);

    printf("[Ctrl Status] rdy 0x%x shst 0x%x\n", _nvme_regs->csts.bits.rdy, _nvme_regs->csts.bits.shst);

    // 2. wait csts.rdy == 0
    while (_nvme_regs->csts.bits.rdy == 0)
    {
        printf("wait for controller shutting down... csts.shst %d cc.shn %d\n",
                _nvme_regs->csts.bits.shst, _nvme_regs->cc.bits.shn);
        sleep(1);
    }

    // 3. set admin qpair
    _admin_sq = allocator->Alloc<NVMeSQ>();
    _admin_cq = allocator->Alloc<NVMeCQ>();

    SetRegs<u64>(offsetof(NVMeRegister, asq), allocator->VirtToPhys(_admin_sq));
    SetRegs<u64>(offsetof(NVMeRegister, acq), allocator->VirtToPhys(_admin_cq));

    // 4. set admin qpair depth


    return true;
}

template <typename T>
void NVMeDriver::SetRegs(u32 offset, T value)
{
    _mm_mfence();
    *(volatile T*)((char*)_nvme_regs + offset) = value;
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
