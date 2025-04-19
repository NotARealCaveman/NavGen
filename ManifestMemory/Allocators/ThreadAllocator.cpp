#include "ThreadAllocator.h"

using namespace Manifest_Memory;
  
MemoryArena& ThreadAllocator::GetMemoryArena()
{
	return memoryArena;
}

MemoryPool& ThreadAllocator::GetMemoryPool()
{
	return memoryPool;
}

const MFsize Manifest_Memory::AlignedSize(const MFsize bytes, const MFsize alignment)
{
	return (bytes + alignment - 1) & ~(alignment - 1);
}