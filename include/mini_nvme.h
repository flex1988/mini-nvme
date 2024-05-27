#pragma once

#include <string>
#include "nvme_spec.h"
#include "mini_nvme.h"

namespace MiniNVMe
{

struct CtrlCap
{
    u32 max_queue_entries;
    bool contiguous_queues_required;
    u16  arbitraction_mechanism_supported;
    u32  timeout;
    u16  doorbell_stride;
    u16  nvm_subsystem_reset_supported;
    u16  command_sets_supported;
    u32  memory_page_size_min;
    u32  memory_page_size_max;
};

struct PCIResource
{
    void*    ptr;
    u64      length;
};

class PCI
{
public:
    bool Init(const std::string& pci_addr);

    uint64_t ReadHexResource(const std::string& attr);

    template<typename T>
    T ReadResource(const std::string& attr, uint64_t offset);

    PCIResource MapResource(const std::string& attr);

private:
    std::string _pci_addr;
};

class NVMeDriver
{
public:
    NVMeDriver();
    ~NVMeDriver();
    bool Init(const std::string& pci_addr);
    bool Start();
    bool Stop();
    bool Deinit();

private:
    PCI*    _pci;
    u32     _nvme_major_version;
    u32     _nvme_minor_version;
    NVMeRegister* _nvme_regs;
    CtrlCap _cap;
};

}
