#pragma once

#include "NavigationMesh.h"
#include "BlackBoardStateUpdater.h"

namespace Manifest_Simulation
{
	struct FlatKey
	{
		MFu64 ID;
		MFu32 index;
		inline MFbool operator==(const FlatKey& other) const { return ID == other.ID; };

		struct Hash
		{
			std::size_t operator()(const FlatKey& key) const
			{
				return std::hash<MFu64>()(key.ID);  // hash by ID
			}
		};
	};

	template <typename T>
	class FlatMap
	{
	public:
		std::unordered_set<FlatKey, FlatKey::Hash> keys;
		std::vector<T> values;
		std::deque<MFu32> freelist;

		MFbool contains(const MFu64 ID) const
		{
			FlatKey key{ ID,0 };
			return keys.contains(key);
		}

		void set(const MFu64 ID, T&& value)
		{
			if (contains(ID))
			{
				const auto key{ keys.find(FlatKey{ID,0}) };
				const MFu32 index{ key->index };
				values.at(index) = std::forward<T>(value);
			}
			else
				insert(std::make_pair(ID, std::forward<T>(value)));			
		}

		void insert(std::pair<MFu64, T>&& insertion)
		{
			auto& [ID, value] { insertion };
			MFu32 index{ !freelist.empty() ? freelist.front() : static_cast<MFu32>(values.size()) };
			FlatKey key{ .ID { ID }, .index{ index } };
			if (index == values.size())
				values.emplace_back(std::move(value));			
			else
			{
				freelist.pop_front();
				values.at(index) = std::move(value);
			}

			keys.insert(key);
		}

		void erase(const MFu64 ID)
		{
			if (!contains(ID))
				return; 

			FlatKey key{ ID,0 };
			key = *keys.find(key);
			keys.erase(key);
			freelist.push_back(key.index);
		}

		T& at(const MFu64 ID)
		{
			assert(contains(ID));
			FlatKey key{ ID,0 };
			key = *keys.find(key);
			const MFu32 valueIndex{ key.index };
			return values.at(valueIndex);
		}

		const T& at(const MFu64 ID) const
		{
			assert(contains(ID));
			FlatKey key{ ID,0 };
			key = *keys.find(key);
			const MFu32 valueIndex{ key.index };
			return values.at(valueIndex);
		}
		 
		typename std::vector<T>::iterator begin() { return values.begin(); }
		typename std::vector<T>::iterator end() { return values.end(); }
		typename std::vector<T>::const_iterator begin() const { return values.begin(); }
		typename std::vector<T>::const_iterator end() const { return values.end(); }
	};

	template<typename T>
	class SparseMap
	{
	public:
		std::unordered_set<FlatKey, FlatKey::Hash> symbols;
		std::vector<FlatMap<T>> maps;

		MFbool contains(const Representation representation) const
		{
			return symbols.contains(FlatKey{ .ID { representation }, .index { 0 } });
		}

		void insert(const Representation representation)
		{
			if (symbols.contains(FlatKey{ .ID { representation }, .index { 0 } }))
				return;

			symbols.insert(FlatKey{ .ID {representation },.index{static_cast<MFu32>(maps.size())} });
			maps.emplace_back();
		}

		FlatMap<T>& at(const Representation representation)
		{
			assert(contains(representation));

			const auto symbolIterator{ symbols.find(FlatKey{.ID{representation}, .index{0}}) };

			return maps.at(symbolIterator->index);
		}

		const FlatMap<T>& at(const Representation representation) const
		{
			assert(contains(representation));

			const auto symbolIterator{ symbols.find(FlatKey{.ID{representation}, .index{0}}) };

			return maps.at(symbolIterator->index);
		}
	};


	struct BlackBoard
	{
	public:
		void Update();
		void InsertObject(const Descriptor _objectArchetypes, const Descriptor _objectCapabilities, const MFu64 objectID);
		const WorldState GetState(const Representation representation, const  MFu64 objectID) const;
		void SetState(const WorldState state, const Representation representation, const  MFu64 objectID);
		inline MFbool HasState(const Representation representation) const { return objectStates.contains(representation); };
		inline void InsertState(const Representation representation) { objectStates.insert(representation); };
		inline const FlatMap<Descriptor>& GetObjectCapabilities() { return objectCapabilities; };
		inline const FlatMap<Descriptor>& GetObjectAttributes(const Representation representation) const { return objectAttributes; };
		template<typename T, typename Deleter>
		void InsertUpdateTable(const MFu64 tableSymbol, Manifest_Persistence::Table<T, Deleter>* stateTable)
		{
			if (tableStateUpdaters.contains(tableSymbol))
				return;

			TableStateUpdater tableStateUpdater;
			tableStateUpdater.CreateUpdateTable(stateTable);
			tableStateUpdaters.insert(std::make_pair(tableSymbol, std::move(tableStateUpdater)));
		};
		template<typename T, typename Deleter>
		void InsertTableUpdateFunction(const MFu64 tableSymbol, const MFu64 stateSymbol, StateUpdateFunction<T, Deleter> updateFunction)
		{
			ASSERT(tableStateUpdaters.contains(tableSymbol));
			if (objectStates.contains(stateSymbol))
				return;
			TableStateUpdater& tableStateUpdater{ tableStateUpdaters.at(tableSymbol) };
			tableStateUpdater.InsertStateUpdateFunction<T, Deleter>(stateSymbol, updateFunction);
		};

		NavigationMesh navigationMesh;	
		SparseMap<WorldState> objectStates;	
		FlatMap<TableStateUpdater> tableStateUpdaters;
		FlatMap<Descriptor> objectAttributes;
		FlatMap<Descriptor> objectCapabilities;
	};
}