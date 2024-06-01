#include "mini_nvme.h"
#include <vector>
#include <map>

namespace MiniNVMe
{

struct DmaChunk
{
    void*  ptr;
    u64 length;
    u64 allocated_size;
    u64 freed_size;
};

class DmaAllocator
{
public:
    DmaAllocator(const char* path, u32 capacity, u32 chunk_size)
    : _hugepage_path(path), _capacity(capacity), _available_chunk(nullptr), _chunk_size(chunk_size), _alloc_chunk_id(0)
    {
    }

    void Init();
    void* Alloc(u64 size, u32 alignment = sizeof(u64));
    void Free(void*); 
    u64 VirtToPhys(void* virt);

private:
    void allocNewChunk();

    std::string            _hugepage_path;
    u32                    _capacity;
    DmaChunk*              _available_chunk;
    u32                    _chunk_size;
    u32                    _alloc_chunk_id;
    u32                    _page_size;
    std::map<u64, DmaChunk*> _used_chunks;
};

}
