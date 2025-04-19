#pragma once
#include <ManifestMemory/Allocators/GlobalAllocator.h>
#include <deque>

using Manifest_Memory::GlobalAllocator;

//to be moved to core path
namespace Manifest_Core
{	
	template<typename T>
	struct Spawner
	{  			
		Spawner() : spawnedObjects{ &GlobalAllocator::ThreadMemoryPool() }
		{};				

		T& Spawn(MFu32& spawnIndex)
		{
			spawnIndex = spawnedObjects.size();
			T& spawnedObject{ spawnedObjects.emplace_back() };
			resource = spawnedObjects.data();
			return spawnedObject;
		}		
		T& Spawn(MFu32& spawnIndex, std::deque<MFu32>& freeList)
		{
			spawnIndex = freeList.front();
			freeList.pop_front();
			T& spawnedObject{ spawnedObjects.at(spawnIndex) };

			return spawnedObject;
		}
		T& GetSpawnedObject(const MFu32 spawnIndex)
		{
			return spawnedObjects.at(spawnIndex);
		}
		T* data() const
		{
			return resource;
		}
	private:
		std::pmr::vector<T> spawnedObjects;		
		T* resource;
	};	 

	template<typename Base, typename Derived>
	concept IsDerivedFrom = std::is_base_of_v<Base, Derived>;
}
 