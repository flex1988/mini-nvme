#include "gtest/gtest.h"
#include "nvme_spec.h"
#include "memory.h"

using namespace MiniNVMe;

TEST(DmaAllocator, Alloc)
{
    MiniNVMe::DmaAllocator* allocator = new MiniNVMe::DmaAllocator("/mnt/huge", 1 << 32, 1 << 20);
    void* ptr = allocator->Alloc<u64>();
    ASSERT_NE(ptr, nullptr);
    printf("virt %p phys %p\n", ptr, allocator->VirtToPhys(ptr));
    memset(ptr, 0, sizeof(u64));
}

GTEST_API_ int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
