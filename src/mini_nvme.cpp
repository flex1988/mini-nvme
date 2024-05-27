#include "mini_nvme.h"
#include "memory.h"

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
    
    _cap.max_queue_entries = _nvme_regs->cap.mqes;
    _cap.timeout = 500 * _nvme_regs->cap.to;
    _cap.doorbell_stride = 1 << (2 + _nvme_regs->cap.dstrd);
    _cap.memory_page_size_min = 1 << (12 + _nvme_regs->cap.mpsmin);
    _cap.memory_page_size_max = 1 << (12 + _nvme_regs->cap.mpsmax);
    printf("[Controller Capabilities] MQES %d Timeout %dms DSTRD %d bytes MPSMIN %d bytes MPSMAX %d bytes\n",
            _cap.max_queue_entries + 1, _cap.timeout, _cap.doorbell_stride, _cap.memory_page_size_min, _cap.memory_page_size_max);

    _nvme_major_version = _nvme_regs->vs.mjr;
    _nvme_minor_version = _nvme_regs->vs.mnr;
    printf("[NVMe specification] %u.%u\n", _nvme_major_version, _nvme_minor_version);

    printf("[Controller Configuration] en %d css %d mps %d iosqes %d iocqes %d\n",
            _nvme_regs->cc.en, _nvme_regs->cc.css, _nvme_regs->cc.mps, _nvme_regs->cc.iosqes, _nvme_regs->cc.iocqes);

    AQARegister* aqa = (AQARegister*)&_nvme_regs->aqa;
    printf("[AQA] ACQS %d ASQS %d\n", aqa->asqs, aqa->acqs);
    
    ASQRegister* asq = (ASQRegister*)&_nvme_regs->asq;
    printf("[ASQ] ASQB %x\n", asq->asqb);

    ACQRegister* acq = (ACQRegister*)&_nvme_regs->acq;
    printf("[ACQ] ACQB %x\n", acq->acqb);

    MiniNVMe::DmaAllocator* allocator = new MiniNVMe::DmaAllocator("/mnt/huge", 1 << 30, 32 << 20);
    void* a = allocator->Alloc(32);

    return true;
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
