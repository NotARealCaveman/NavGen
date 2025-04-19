#pragma once
#include <vector>

#include <ManifestSimulation/CollisionEngine/Colliders/AxisAlignedBoxes.h>

#include "Extraction/Transvoxel.h"
#include "Extraction/ExtractionData.h"

using Manifest_Simulation::AxisBoundingBox;

namespace Manifest_Terrain
{
	struct TerrainExtractionData
	{
		PrimaryTerrain primaryTerrain;
		TransitionTerrain transitionTerrain[NUMBER_TRANSITION_FACES];
		//used by systems that read from terrain 
		AxisBoundingBox terrainCollider;
		MFpoint3 wsMinCorner;
		MFpoint3 vsMinCorner;
		Integer3D extractionCorner;
	};
	void PrepareTempTerrainExtractionData(TerrainExtractionData& tempTerrainExtractionData);
	void CopyTempTerrainExtractionData(const TerrainExtractionData& tempTerrainExtractionData, TerrainExtractionData& finalTerrainExtractionData);
	void ResetTempTerrainExtractionData(TerrainExtractionData& tempTerrainExtractionData);

	//worldSpaceZoffset is the max grid "y"(z) value as -z is forward
	void ExtractTerrainData(const Integer3D& minCorner, const VoxelMap& map, const MFdouble worldSpaceZoffest, const MFu8 extractionLOD, const MFu8 worldLOD, TerrainExtractionData& extractionData);

	struct TerrainExtraction
	{		
		std::vector<TerrainExtractionData> data;
		std::vector<MFu32> updatedChunks;
		Grid2D worldGrid;		
		MFsize totalExtractedChunks{ 0 };
		MFu8 maxExtractionLOD{ MAX_TERRAIN_LOD_SUPPORT };
		MFfloat voxelSpaceZOffset{ 0 };
	};

	//reclaims removed terrain chunks via their deleter
	struct TerrainExtractionDeleter
	{
		void operator()(TerrainExtraction* terrainExtraction)
		{			
			/*removed terrain chunks - will look at terrain allocations later once custom allocator is up and running*/
		}
	};	
}