#pragma once
#include <chrono>
#include <vector>	
#include <functional>
#include <string>

#include <ManifestMemory/Manifest_Allocator.h>
#include <EXPERIMENTAL/EXPERIMENTAL_RUNTIME_DATA_STRUCTURES.h>

#include <ManifestPersistence/Runtime/Runtime_TableTypes.h>
#include "Binary_Database.h"

using namespace Manifest_Memory;
using namespace Manifest_Experimental;

namespace Manifest_Persistence
{
	//Currently exploring a push/pull paradigm for updating and centralizing shared game state in the runtime database
	class ManifestRuntimeDatabase
	{
		public:
			ManifestRuntimeDatabase();
			SimulationStateTable* simulationStateTable;
			TerrainExtractionTable* terrainExtractionTable;
			WorldConfigurationTable* worldConfigurationTable;		
			ActionPlanTable* actionPlanTable;
	};
}

