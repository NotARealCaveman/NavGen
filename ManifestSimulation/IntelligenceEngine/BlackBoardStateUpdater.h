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
	//returns the hash of the symbol in all upper case
	class SymbolHash
	{
	public:
		constexpr SymbolHash(const char* symbol) : hash{ HashFnv1a(symbol) } {}
				
		constexpr operator MFu64() const { return hash; }

	private:
		static constexpr MFu64 FNV1A_PRIME = 1099511628211ull;
		static constexpr MFu64 FNV1A_BASIS = 14695981039346656037ull;

		static constexpr char ToUpperChar(char c) {
			return (c >= 'a' && c <= 'z') ? (c - 'a' + 'A') : c;
		}

		static constexpr MFu64 HashFnv1a(const char* str, MFu64 hash = FNV1A_BASIS) {
			return (*str) ? HashFnv1a(str + 1, (hash ^ static_cast<MFu64>(ToUpperChar(*str))) * FNV1A_PRIME) : hash;
		}

		const MFu64 hash;
	};

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