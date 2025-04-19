#pragma once

#include <ManifestMath/Core.h> 
using namespace Manifest_Math;

 
namespace Manifest_Terrain
{ 
	constexpr MFu8 MAX_TERRAIN_LOD_SUPPORT = 3;
	constexpr MFsize MAX_TERAIN_CHUNK_SUPPORT = 36;
	constexpr MFu8 WORLD_INTERPOLATION = 8;	
	constexpr MFint32 BASE_CELLS_PER_BLOCK = 16;
	constexpr MFint32 BLOCK_SIZE_VOXEL_SPACE = (1 << WORLD_INTERPOLATION);
	constexpr MFdouble WORLD_SCALE = 0.01;
	constexpr MFdouble BLOCK_SIZE_WORLD_SPACE = BLOCK_SIZE_VOXEL_SPACE *WORLD_SCALE;	 
}
