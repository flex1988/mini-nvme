#include "stdio.h"
#include "mini_nvme.h"

int main(int argc, char** argv) {
    if (argc < 2)
    {
        printf("not enough arguments\n");
        return -1;
    }
    std::string pci_addr = argv[1];
    printf("pci addr %s\n", pci_addr.c_str());
    MiniNVMe::NVMeDriver* driver = new MiniNVMe::NVMeDriver;
    driver->Init(pci_addr);

    // MiniNVMe::NVMeDevice* device = driver->CreateDevice();
    // device->Write();
    
    return 0;
}
