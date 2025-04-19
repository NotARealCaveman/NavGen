#include "GlobalAllocator.h"

using namespace Manifest_Memory;

std::array<ThreadAllocator, MAX_THREAD_SUPPORT> GlobalAllocator::allocators;

///THREAD MEMORY ACCESSING

MemoryPool& GlobalAllocator::ThreadMemoryPool()
{
	ThreadAllocator& threadAllocator{ GetThreadAllocator() };
	
	return threadAllocator.GetMemoryPool();
}

MemoryArena& GlobalAllocator::ThreadMemoryArena()
{
	ThreadAllocator& threadAllocator{ GetThreadAllocator() };

	return threadAllocator.GetMemoryArena();
}

//wrapper around functionality to get(and on first visit register) the thread allocator owned by this thread
ThreadAllocator& GlobalAllocator::GetThreadAllocator()
{
	std::thread::id threadID{ std::this_thread::get_id() };

	const MFu8 allocatorIndex{ GetThreadAllocatorIndex(threadID) };
	assert(allocatorIndex < MAX_THREAD_SUPPORT, "TOO MANY THREAD ALLOCATIONS");
	ThreadAllocator& threadAllocator{ allocators.at(allocatorIndex) };

	return threadAllocator;
}

//searches for an allocator that contains an owner id matching the thread
//if the id is encountered the index is returned
//if no id is encountered, thread is unregistered and attemps to register
//if max support is cycled through, no room is left for thread - error out
MFu8 GlobalAllocator::GetThreadAllocatorIndex(const std::thread::id threadID)
{
	MFu8 result{ 0 };
	std::thread::id noOwner;
	while (result < MAX_THREAD_SUPPORT)
	{
		std::thread::id threadOwner{ allocators.at(result).ownerID.load(std::memory_order_relaxed) };

		//thread was found
		if (threadOwner == threadID)
			return result;
		//end of registered threads 
		else if (threadOwner == noOwner)
			return RegisterThread(threadID, result);

		++result;
	}

	return MAX_THREAD_SUPPORT;
}

//Attempts to register a thread upon it's first visit should room exist
const MFu8 GlobalAllocator::RegisterThread(const std::thread::id threadID, const MFu8 initialEndIndex)
{
	MFu8 result{ initialEndIndex };	
	while (result < MAX_THREAD_SUPPORT)
	{
		std::thread::id noOwner;
		const MFbool registered{ allocators.at(result).ownerID.compare_exchange_weak(noOwner,threadID,std::memory_order_acq_rel,std::memory_order_relaxed) };

		if (registered)
		{
			DLOG({ CONSOLE_BG_GREEN }, "Thread:", threadID, " registered to index:", +result);
			return result;
		}		

		++result;
	}

	return MAX_THREAD_SUPPORT;
}
