#pragma once
#include <atomic>
#include <array>
#include <algorithm>
#include <ranges>

#include <ManifestUtility/Typenames.h>
#include <ManifestUtility/DebugLogger.h>

namespace Manifest_Memory
{		
	using Generation = MFu32;
	constexpr Generation DEFAULT_GENERATION{ 0 };
	constexpr Generation MAX_RCU_GENERATION{ 2 };	
	constexpr MFu32 RCU_MODULO{ MAX_RCU_GENERATION - 1 };


	//ideally read_lock can be changed to an if statement instead of the while loop shoudl the guarantee of a single writer be held. once the read flag is taken on a generation that gets updated, the new generation flag is now taken which keeps the writer locked in the wait loop until the store of the false on the old generation. ordering should also prevent the old from being set before the current. This should make the readers of any generation wait free 
	template<typename T, typename Deleter>
	class RCU
	{
	private:						
		struct ReadFlag
		{
			std::atomic<MFbool> isReading{ false };
			const MFu8 padding[64 - sizeof(isReading)]{};
		};
		struct GenerationHandle
		{
			Generation generation{ DEFAULT_GENERATION };
			T* handle;
		};
		//registration
		const MFsize maxReaders;
		std::atomic<MFu32> registeredReaders;

		//rcu				
		Deleter deleter;		
		//GenerationHandle generationHandles[MAX_RCU_GENERATION];
		std::array<GenerationHandle, MAX_RCU_GENERATION> generationHandles;
		std::array<ReadFlag*, MAX_RCU_GENERATION> generationReadFlags;
		std::atomic<Generation> globalGeneration;		
	public:
		using Handle = GenerationHandle;
		RCU() = default;
		RCU(const MFsize _maxReaders)
			: maxReaders(_maxReaders), registeredReaders{ 0 }, deleter{ Deleter{} }, globalGeneration{ DEFAULT_GENERATION }
		{
			//create default generation handles
			std::ranges::for_each(generationHandles, [](GenerationHandle& generationHandle)->void
				{
					generationHandle = GenerationHandle{ DEFAULT_GENERATION,new T };
				});
			//create default read flags
			std::ranges::for_each(generationReadFlags, [=](ReadFlag*& readFlag)->void
				{
					readFlag = new ReadFlag[maxReaders];
				});
		}	
		const GenerationHandle rcu_read_lock(const MFu32& readerId)
		{
			//use current generation as guess
			Generation currentGeneration = globalGeneration.load(std::memory_order_relaxed);
			MFu32 generationIndex = currentGeneration & RCU_MODULO;
			//block writer from deleting from index
			generationReadFlags[generationIndex][readerId].isReading.store(true, std::memory_order_release);
			Generation oldGeneration = currentGeneration;
			//if guess was invalid - remove block and try again			
			while(oldGeneration != (currentGeneration = globalGeneration.load(std::memory_order_acquire)))
			{
				DLOG({ CONSOLE_RED }, "inconsistent state detected!");
				MFu32 oldIndex = generationIndex;
				generationIndex = currentGeneration & RCU_MODULO;				
				generationReadFlags[generationIndex][readerId].isReading.store(true, std::memory_order_release);
				generationReadFlags[oldIndex][readerId].isReading.store(false, std::memory_order_relaxed);//won't be reordered above release
				oldGeneration = currentGeneration;
			}
			//return a handle to the read-locked generation 
			return generationHandles[generationIndex];
		}
		void rcu_read_unlock(const GenerationHandle& generationHandle, const MFu32& readerId)
		{
			//unblock read generation for writer
			const MFu32 generationIndex = generationHandle.generation & RCU_MODULO;
			generationReadFlags[generationIndex][readerId].isReading.store(false, std::memory_order_relaxed);
		}
		void synchronize_rcu(T* update)
		{
			//store old generation for current readers;
			const Generation oldGeneration = globalGeneration.load(std::memory_order_relaxed);
			const MFu32 oldIndex = oldGeneration & RCU_MODULO;;
			//prepare new geneartion for future readers
			const Generation newGeneration = oldGeneration + 1;
			const Handle newHandle{ newGeneration,update };
			const MFu32 newIndex = newGeneration & RCU_MODULO;
			//make new generation visible for future readers
			generationHandles[newIndex] = newHandle;
			globalGeneration.store(newGeneration, std::memory_order_release);
			//wait for old readers
			MFu32 waitCount{ 0 };		
			for (auto reader{ 0 }; reader < registeredReaders.load(std::memory_order_relaxed); ++reader)
				while (generationReadFlags[oldIndex][reader].isReading.load(std::memory_order_acquire))
				{
					_mm_pause();
					++waitCount;		
				}
			if(waitCount)
				LOG({ CONSOLE_YELLOW }, "Waiting(", waitCount, ")");
			//release unused memory
			deleter(generationHandles[oldIndex].handle);
			generationHandles[oldIndex].handle = nullptr;
		}
		MFu32 RegisterReader()
		{
			assert(registeredReaders.load(std::memory_order_seq_cst) < maxReaders);
			return registeredReaders.fetch_add(1, std::memory_order_relaxed);
		}
	};
}