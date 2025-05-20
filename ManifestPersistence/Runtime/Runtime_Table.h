#pragma once
#include <functional>

#include <ManifestMemory/MemoryGuards/RCU.h>
#include <ManifestMemory/MemoryGuards/ExchangeLock.h>
#include <ManifestUtility/TypeAssist.h>

using namespace Manifest_Utility;
using namespace Manifest_Memory;

namespace Manifest_Persistence
{
	template<typename T,typename Deleter>
	class Table
	{
	private:
		using RCU = RCU<T, Deleter>;
				
		RCU rcu;	
		ExchangeLock writeLock;
	public:			
		Table(const MFsize maxConcurrentReaders)
			: rcu{ maxConcurrentReaders } {};				

		template<typename Function, typename... Params>
		typename std::enable_if_t<!std::is_same_v<Manifest_Utility::ReturnType<Function, Params...>, void>, void>  Push(Function&& function, Params&&... params)
		{		
			writeLock.Lock();
			rcu.synchronize_rcu(Manifest_Utility::ForwardFunction(function, params...));
			writeLock.Unlock();
		};
		
		template<typename Function, typename... Params>
		using TableReturnType = Manifest_Utility::ReturnType<Function, decltype(std::declval<typename RCU::Handle>()), Params...>;
			
		//return type pull - for types that can be internally constructed 
		template<typename Function, typename... Params>
		typename std::enable_if_t<!std::is_same_v<TableReturnType<Function, Params...>, void>, TableReturnType<Function, Params...>> Pull(const MFu32 readerId, Function&& function, Params&&... params)
		{
			const typename RCU::Handle handle = rcu.rcu_read_lock(readerId);
			auto&& result{ Manifest_Utility::ForwardFunction(function, handle, params...) };
			rcu.rcu_read_unlock(handle, readerId);

			return std::move(result);
		};

		//void specialized pull - for types that need external constructions
		template<typename Function, typename... Params>
		typename std::enable_if_t<std::is_same_v<TableReturnType<Function,Params...>,void>,void> Pull(const MFu32 readerId, Function&& function, Params&&... params)
		{
			typename RCU::Handle handle = rcu.rcu_read_lock(readerId);
			Manifest_Utility::ForwardFunction(function, handle, params...);
			rcu.rcu_read_unlock(handle, readerId);
		};
		//if successful returns an index reserved for reader
		inline MFu32 ReserveTableReadFlag()
		{
			return rcu.RegisterReader();
		}
	};
}
