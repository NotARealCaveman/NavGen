#pragma once

#include <ManifestUtility/DebugLogger.h>

#include <ManifestMath/Core.h>
#include <ManifestMath/Point3.h>
#include <ManifestMath/PerlinNoise.hpp>
#include <ManifestMath/Integer3D.h>

using namespace Manifest_Math;

namespace Manifest_Terrain
{
	using Voxel = MFint8;

	struct VoxelMap
	{		
		MFu32 nVoxels;
		MFu32 mVoxels;	
		MFu32 hVoxels;
		MFu8 lod;			

		Voxel* field;
	};

	//generates a voxel map large enough for the requested number of blocks per axis and a max lod level
	VoxelMap GenerateVoxelMap(const MFu32& seed, const MFu8& lod, const MFu32& nBlocks, const MFu32& mBlocks, const MFu32& hBlocks, const Integer3D& mapMinCorner);

	//DENSITY FUNCTIONS
	//flat plane
	static constexpr MFint32 groundLevel = 2;	
	static constexpr MFint32 ceilingLevel = groundLevel + 24;
	static inline Voxel DensityFunction_0(siv::PerlinNoise& noise, const MFu32 i, const MFu32& j, const MFu32& k);
	static inline Voxel DensityFunction_00(siv::PerlinNoise& noise, const MFu32 i, const MFu32& j, const MFu32& k);
	//random 3d noise
	static inline Voxel DensityFunction_1(siv::PerlinNoise& noise, const MFu32 i, const MFu32& j, const MFu32& k);
	//terrain func 1
	static inline Voxel DensityFunction_2(siv::PerlinNoise& noise, const MFu32 i, const MFu32& j, const MFu32& k);
	//terrain func 2
	static inline Voxel DensityFunction_3(siv::PerlinNoise& noise, const MFu32 i, const MFu32& j, const MFu32& k);
}