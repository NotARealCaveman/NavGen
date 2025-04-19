#pragma once
#include <ranges>
#include <future>
#include <chrono>

#include "ThreadAllocator.h"

#include <ManifestUtility/DebugLogger.h>

namespace Manifest_Memory
{    
    constexpr MFu8 MAX_THREAD_SUPPORT{ 5 };
    struct GlobalAllocator
    { 
        static MemoryPool& ThreadMemoryPool();           
        static MemoryArena& ThreadMemoryArena();
    private:        
        //Returns the Thread Allocator owned by the calling thread ID
        static ThreadAllocator& GetThreadAllocator();
        //Invokes RegisterThread() if thread has no allocator Index
        static MFu8 GetThreadAllocatorIndex(const std::thread::id threadID);
        //called the first time a thread calls into the global allocator
        //returns the thread index the thread owner belongs to 
        static const MFu8 RegisterThread(const std::thread::id threadID, const MFu8 initialEndIndex);

        static std::array<ThreadAllocator, MAX_THREAD_SUPPORT> allocators; 
    };
}