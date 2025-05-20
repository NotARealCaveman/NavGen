#pragma once
#include <chrono>
#include <vector>	
#include <functional>
#include <string> 
#include <ManifestPersistence/Runtime/Runtime_TableTypes.h>

using namespace Manifest_Memory;

namespace Manifest_Persistence
{
	//Currently exploring a push/pull paradigm for updating and centralizing shared game state in the runtime database
	struct ManifestRuntimeDatabase
	{
		public: 			
			TerrainExtractionTable* terrainExtractionTable;			
			ActionPlanTable* actionPlanTable;
	};
}

