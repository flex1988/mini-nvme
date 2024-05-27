#include "mini_nvme.h"
#include <vector>

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

    void* Alloc(u64 size);
    void Free(void*); 

private:
    std::string            _hugepage_path;
    u32                    _capacity;
    DmaChunk*              _available_chunk;
    std::vector<DmaChunk*> _used_chunks;
    u32                    _chunk_size;
    u32                    _alloc_chunk_id;
};

}