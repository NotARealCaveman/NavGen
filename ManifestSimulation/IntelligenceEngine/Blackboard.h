#pragma once

#include <ManifestMemory/Containers/SparseMap.h>
#include <ManifestUtility/SymbolHash.h>

#include "NavigationMesh.h"
#include "BlackBoardStateUpdater.h"

namespace Manifest_Simulation
{
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