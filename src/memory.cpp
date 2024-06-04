#include "memory.h"
#include "fcntl.h"
#include "assert.h"
#include <unistd.h>
#include <sys/mman.h>

namespace MiniNVMe
{

void DmaAllocator::Init()
{
    _page_size = sysconf(_SC_PAGESIZE);
}

void DmaAllocator::allocNewChunk()
{
    char data[32];
    int ret = snprintf(data, 32, (_hugepage_path + "/mnvme_%d").c_str(), _alloc_chunk_id++);
    assert(ret > 0);
    int fd = open(data, O_RDWR | O_CREAT);
    assert(fd >= 0);
    void* ptr = mmap(nullptr, _chunk_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    assert(ptr != nullptr);

    _available_chunk = new DmaChunk;
    _available_chunk->ptr = ptr;
    _available_chunk->length = _chunk_size;
    _available_chunk->allocated_size = 0;
    _available_chunk->freed_size = 0;
}

void DmaAllocator::Free(void* ptr)
{
    u64 chunkid = (u64)ptr / _chunk_size;
    if (_used_chunks.find(chunkid) == _used_chunks.end())
    {
        std::abort();
    }
    // _used_chunks[chunkid]->freed_size -= 
    
}


u64 DmaAllocator::VirtToPhys(void* virt)
{
    int fd = open("/proc/self/pagemap", O_RDONLY);
    lseek(fd, (u64)virt / _page_size * sizeof(u64), SEEK_SET);
    u64 addr = 0;
    read(fd, &addr, sizeof(u64));
    close(fd);
    return (addr & 0x7fffffffffffffull) * _page_size + (u64)virt % _page_size;
}

}
