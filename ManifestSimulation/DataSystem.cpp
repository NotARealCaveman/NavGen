#include "DataSystem.h"

using namespace Manifest_Simulation;
 
DataSystem::DataSystem()
	:IDtoIndexMap{}, freeList{  }
{

}

MFu32 DataSystem::GetIDIndex(const MFu64 ID) const
{
	return IDtoIndexMap.contains(ID) ?  IDtoIndexMap.at(ID) : ID_NOT_MAPPED;
}

const std::pmr::unordered_map<MFu64, MFu32>& DataSystem::GetIDIndexMap()
{
	return IDtoIndexMap;
}

const MFsize DataSystem::size() const
{
	return activeSize;
}

void DataSystem::erase(const std::vector<MFu64>& removedIDs)
{
	std::ranges::for_each(removedIDs, [this](const MFu64 removedID)
	{
		IDtoIndexMap.erase(removedID);
	});
	std::ranges::copy(removedIDs, std::back_inserter(freeList));
}
 