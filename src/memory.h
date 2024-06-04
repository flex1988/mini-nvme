#include "mini_nvme.h"
#include "assert.h"
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

    template<typename T>
    T* Alloc(u32 count = 1)
    {
        u32 alignment = alignof(T);
        u32 size = sizeof(T);
        assert(size >= alignment);
        assert(size % alignment == 0);
        void* ptr = nullptr;
        if (_available_chunk == nullptr)
        {
            allocNewChunk();
        }
        else if (_chunk_size - _available_chunk->allocated_size < size * count)
        {
            _used_chunks.insert({ (u64)_available_chunk->ptr / _chunk_size,  _available_chunk });
            _available_chunk = nullptr;
            allocNewChunk();   
        }
        
        ptr = (char*)_available_chunk->ptr + _available_chunk->allocated_size;
        
        if (reinterpret_cast<u64>(ptr) & (alignment - 1))
        {
            u32 unaligned = (alignment - (reinterpret_cast<u64>(ptr) & (alignment - 1)));
            ptr += unaligned;
            _available_chunk->allocated_size += unaligned;
        }

        _available_chunk->allocated_size += size * count;
        return (T*)ptr;
    }

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
