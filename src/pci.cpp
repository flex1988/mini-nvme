#include "mini_nvme.h"

#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <sys/stat.h>
#include <sys/mman.h>
#include "stdio.h"
#include "stdint.h"
#include "string.h"
#include "assert.h"

namespace MiniNVMe
{

uint64_t PCI::ReadHexResource(const std::string& attr)
{
    char path[64];
    snprintf(path, 64, "/sys/bus/pci/devices/%s/%s", _pci_addr.c_str(), attr.c_str());
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        printf("open pci %s failed error: %s\n", path, strerror(errno));
        return -1;
    }
    char buf[8];
    read(fd, buf, 2);
    assert(buf[0] == '0');
    assert(buf[1] == 'x');
    read(fd, buf, 4);
    uint64_t value = std::stoi(buf, 0, 16);
    return value;
}

PCIResource PCI::MapResource(const std::string& attr)
{
    char path[64];
    snprintf(path, 64, "/sys/bus/pci/devices/%s/resource0", _pci_addr.c_str());
    struct stat s;
    if (stat(path, &s) != 0)
    {
        printf("stat pci %s resouce failed: %s\n", path, strerror(errno));
        return { nullptr, 0 };
    }
    int fd = open(path, O_RDWR);
    if (fd < 0)
    {
        printf("open pci %s failed error: %s\n", path, strerror(errno));
        return { nullptr, 0 };
    }
    void* ptr = mmap(NULL, s.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED)
    {
        printf("pci mmap resouce failed %s\n", path);
        return { nullptr, 0 };
    }
    return { ptr, s.st_size };
}

template<typename T>
T PCI::ReadResource(const std::string& attr, uint64_t offset)
{
    char path[64];
    snprintf(path, 64, "/sys/bus/pci/devices/%s/%s", _pci_addr.c_str(), attr.c_str());
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        printf("open pci %s failed error: %s\n", path, strerror(errno));
        return -1;
    }
    lseek(fd, offset, SEEK_SET);
    
    T value;
    read(fd, &value, sizeof(T));
    return value;
}

bool PCI::Init(const std::string& pci_addr)
{
    _pci_addr = pci_addr;
    uint64_t vendor = ReadHexResource("vendor");
    uint64_t device = ReadHexResource("device");
    uint32_t class_id = ReadResource<uint32_t>("config", 8) >> 16;

    printf("pci init vendor 0x%x device 0x%x class 0x%x\n", vendor, device, class_id);

    if (class_id != 0x0108)
    {
        printf("device %s is not a block device\n", pci_addr.c_str());
        return false;
    }

    return true;    
}

}
