#include "Voxel.h"

using namespace Manifest_Terrain;


VoxelMap Manifest_Terrain::GenerateVoxelMap(const MFu32& seed, const MFu8& lod, const MFu32& nBlocks, const MFu32& mBlocks, const MFu32& hBlocks, const Integer3D& mapMinCorner)
{
	auto noise = siv::PerlinNoise{ seed };
	const MFu32& nVoxels = (19 + (nBlocks - 1) * 17) << lod;
	const MFu32& mVoxels = (19 + (mBlocks - 1) * 17) << lod;
	const MFu32& hVoxels = (19 + (hBlocks - 1) * 17) << lod;
	VoxelMap result{ nVoxels,mVoxels,hVoxels,lod };
	result.field = new Voxel[result.nVoxels * result.mVoxels * result.hVoxels];
	for (MFu32 k = 0; k < result.hVoxels; ++k)
		for (MFu32 j = 0; j < result.mVoxels; ++j)
			for (MFu32 i = 0; i < result.nVoxels; ++i)
			{
				result.field[i + result.nVoxels * (j + result.mVoxels * k)] = DensityFunction_0(noise, mapMinCorner.i+i, mapMinCorner.j+j, mapMinCorner.k+ k);
			}
	return result;
}


Voxel Manifest_Terrain::DensityFunction_0(siv::PerlinNoise& noise, const MFu32 i, const MFu32& j, const MFu32& k)
{
	if (k < groundLevel)
		return -1;
	else if (k == groundLevel)
		return 0;
	else
		return 1;

}

Voxel Manifest_Terrain::DensityFunction_00(siv::PerlinNoise& noise, const MFu32 i, const MFu32& j, const MFu32& k)
{	
	const auto x = 25;
	const auto y = 25;
	const auto z = 15;
	auto spherePoint = [&]()
	{
		return (x - i) * (x - i) + (y - j) * (y - j) + (z - k) * (z - k);
	};
	const auto r = 15;
	auto p = spherePoint();
	if (p > r)
		return 1;
	else if (p < r)
		return -1;
	
	return 0;
}


Voxel Manifest_Terrain::DensityFunction_1(siv::PerlinNoise& noise, const MFu32 i, const MFu32& j, const MFu32& k)
{
	if (k > ceilingLevel)
		return 1;
	auto sample = 0;
	sample += noise.noise2D(0.5 * i * 0.00025, 0.5 * j * 0.00025);
	return  sample * 127;
}

Voxel Manifest_Terrain::DensityFunction_2(siv::PerlinNoise& noise, const MFu32 i, const MFu32& j, const MFu32& k)
{	
	if (k < groundLevel)
		return -1;
	if (k > 25)
		return 1;
	MFint8 sample = 127 * noise.noise3D(i * 0.00925, j * 0.00925, k * 0.015);
	sample += 127 * noise.noise2D(i * 0.00925, (j) * 0.00925);
	return  sample;
};

Voxel Manifest_Terrain::DensityFunction_3(siv::PerlinNoise& noise, const MFu32 i, const MFu32& j, const MFu32& k)
{
	if (k > ceilingLevel)
		return 1;
	if (k < groundLevel)
		return -1;
	if (k == groundLevel)
		return 0;
	MFfloat sample = 0;
	sample += noise.noise2D(i * 0.0025, j * 0.0025);
	sample += noise.octave2D_11(i * 0.0125, j * 0.0125, 2, .5);
	sample += noise.noise3D(i * 0.0025, j * 0.0025, 0.75 * k * 0.0025);
	if(k<5)
		sample += noise.octave2D_11(k * 0.0325, k * 0.0125, 4, .75);
	if(k < 10)
		sample+= noise.octave3D_11(i * 0.0125, j * 0.0125, 0.75 * k * 0.0125,2,.5);
	if( k < 20)
		sample += noise.octave3D_11(i * 0.0125, j * 0.0125, 0.75 * k * 0.0125, 8, .5);
	sample = std::fminf(std::fmaxf(sample, -1), 1);
	return  sample * 127;
};