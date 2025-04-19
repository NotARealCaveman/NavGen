#include "NavigableStructures.h"

using namespace Manifest_Simulation;

void* FreeList_NEW::allocate(const MFsize allocationSize)
{
	void* result{ NO_FREELIST_ENTRY };
	if (openAddressesBySizeMap.contains(allocationSize))
	{
		std::deque<void*>& openAddresses{ openAddressesBySizeMap.at(allocationSize) };
		if (!openAddresses.empty())
		{
			result = openAddresses.front();
			openAddresses.pop_front();
		}
	}
	//DLOG({ CONSOLE_BG_GREEN }, "Returning address:", result, "for allocation size request:", allocationSize);
	return result;
}

void FreeList_NEW::deallocate(const MFsize allocationSize, void* address)
{
	//DLOG({ CONSOLE_BG_CYAN }, "Adding address:", address, "for reuse size request:", allocationSize);
	openAddressesBySizeMap[allocationSize].push_back(address);
} 