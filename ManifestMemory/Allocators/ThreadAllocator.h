#pragma once
#include <atomic>
#include <numeric>
#include <thread>
#include <cassert>

#include <ManifestUtility/Typenames.h>

#include "MemoryPool.h"
#include "MemoryArena.h"

using namespace Manifest_Utility;

namespace Manifest_Memory
{
    struct AllocatedResource
    {
        std::byte* resource{ nullptr };
        AllocatedResource* parentResource{ nullptr };
        MFsize reservedSize{ 0 };
        MFsize allocatedSize{ 0 };
    };

    class ThreadAllocator
    {
    public:        
        MemoryArena& GetMemoryArena();
        MemoryPool& GetMemoryPool();
        //default "noOwner" is 08
        std::atomic<std::thread::id> ownerID;
    private:
                
        std::vector<AllocatedResource> allocatedResources;
        MemoryArena memoryArena;
        MemoryPool memoryPool;
    };

    const MFsize AlignedSize(const MFsize bytes, const MFsize alignment);
}