#include "memory.h"
#include "fcntl.h"
#include "assert.h"
#include <sys/mman.h>

namespace MiniNVMe
{

void* DmaAllocator::Alloc(u64 size)
{
    void* ptr = nullptr;
    if (_available_chunk == nullptr)
    {
        char data[32];
        int ret = snprintf(data, 32, (_hugepage_path + "/mnvme_%d").c_str(), _alloc_chunk_id++);
        assert(ret > 0);
        int fd = open(data, O_RDWR | O_CREAT);
        assert(fd >= 0);
        ptr = mmap(nullptr, _chunk_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        assert(ptr != nullptr);

        _available_chunk = new DmaChunk;
        _available_chunk->length = _chunk_size;
        _available_chunk->allocated_size = 0;
        _available_chunk->freed_size = 0;
        _available_chunk->allocated_size += size;
    }
    return ptr;
}

void DmaAllocator::Free(void* ptr)
{

}

}
