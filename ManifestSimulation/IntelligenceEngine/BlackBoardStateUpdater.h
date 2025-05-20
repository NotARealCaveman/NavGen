#pragma once
#include <utility>
#include <vector>
#include <deque>
#include <set>

#include <ManifestMemory/MemoryGuards/RCU.h>
#include <ManifestMemory/MemoryGuards/ExchangeLock.h>
#include <ManifestPersistence/Runtime/Runtime_Table.h>

#include "WorldState.h"

namespace Manifest_Simulation
{
	

	//testing updater	
	struct BlackBoard;
	template<typename T, typename Deleter>
	using StateUpdateFunction = void(*)(const typename Manifest_Memory::RCU<T, Deleter>::Handle&, BlackBoard&);

	constexpr MFu32 NAV_MESH_UPDATER{ std::numeric_limits<MFu32>::max() };
	struct TableStateUpdater
	{
		struct UpdaterBase
		{
			static BlackBoard* blackBoard;
			virtual void Update() const = 0;
		};
		template<typename T, typename Deleter>
		struct StateUpdater : public UpdaterBase
		{
			StateUpdater() {};
			void Update() const override
			{
				updateTable->Pull(updateTableReaderID, [this](const typename Manifest_Memory::RCU<T, Deleter>::Handle& tableHandle, BlackBoard& blackBoard)
				{
					for (const StateUpdateFunction<T, Deleter>& stateUpdateFunction : this->stateUpdateFunctions)
						ForwardFunction(stateUpdateFunction, std::cref(tableHandle), std::ref(blackBoard));
				}, *blackBoard);
			}
			std::vector<StateUpdateFunction<T, Deleter>> stateUpdateFunctions;
			std::set<MFu64> stateUpdateFields;
			Manifest_Persistence::Table<T, Deleter>* updateTable{ nullptr };
			MFu32 updateTableReaderID;
		};

		TableStateUpdater() = default;
		template<typename T, typename Deleter>
		void CreateUpdateTable(Manifest_Persistence::Table<T, Deleter>* stateTable)
		{
			ASSERT(!updater);
			updater = std::make_unique<StateUpdater<T, Deleter>>();
			StateUpdater<T, Deleter>& stateUpdater = static_cast<StateUpdater<T, Deleter>&>(*updater);
			stateUpdater.updateTable = stateTable;
			stateUpdater.updateTableReaderID = stateTable->ReserveTableReadFlag();
			updaterID = reinterpret_cast<uintptr_t>(stateTable);
		};
		template<typename T, typename Deleter>
		void InsertStateUpdateFunction(const MFu64 stateSymbol, StateUpdateFunction<T, Deleter> updateFunction)
		{
			ASSERT(updater != nullptr);
			StateUpdater<T, Deleter>& stateUpdater = static_cast<StateUpdater<T, Deleter>&>(*updater);
			stateUpdater.stateUpdateFunctions.emplace_back(updateFunction);
			stateUpdater.stateUpdateFields.insert(stateSymbol);
		};

		void Update()
		{
			updater->Update();
		}

		std::unique_ptr<UpdaterBase> updater{ nullptr };
		MFuptr updaterID;//address of the underlying update table				
	};
}