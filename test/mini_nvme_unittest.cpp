#include "gtest/gtest.h"
#include "memory.h"

TEST(DmaAllocator, Alloc)
{
    MiniNVMe::DmaAllocator* allocator = new MiniNVMe::DmaAllocator("/mnt/huge", 1 << 32, 1 << 20);
    void* ptr = allocator->Alloc(128);
    ASSERT_NE(ptr, nullptr);
    printf("virt %p phys %p\n", ptr, allocator->VirtToPhys(ptr));
    memset(ptr, 0, 128);
}

GTEST_API_ int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
