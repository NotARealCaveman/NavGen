#pragma once
#include <ManifestMath/Grid2D.h>
#include <ManifestTerrain/VoxelStructs.h>

using namespace Manifest_Math;
using namespace Manifest_Terrain;

namespace Manifest_Core
{
	struct WorldConfiguration
	{
		Grid2D worldGrid;
		MFu32 xChunks;
		MFu32 zChunks;
		MFu8 worldLOD;
		VoxelMap voxelMap;
	};

	struct WorldConfigurationDeleter
	{
		void operator()(WorldConfiguration* worldConfiguration)
		{			
		};
	};
}