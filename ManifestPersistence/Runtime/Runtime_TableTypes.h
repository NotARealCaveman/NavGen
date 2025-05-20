#pragma once
 
#include <ManifestSimulation/IntelligenceEngine/ActionPlanner.h>
#include <ManifestTerrain/TerrainExtraction.h> 

#include "Runtime_Table.h"

 

namespace Manifest_Persistence
{	 
	using TerrainExtractionTable = Table<Manifest_Terrain::TerrainExtraction, Manifest_Terrain::TerrainExtractionDeleter>; 
	using ActionPlanTable = Table<Manifest_Simulation::ActionPlan, std::default_delete<Manifest_Simulation::ActionPlan>>;
}