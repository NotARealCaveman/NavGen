#pragma once

#include <ManifestPersistence/MDB/MDB_VoxelMap.h>

namespace Manifest_Persistence
{
	struct Binary_VoxelMap
	{
		struct Entry_Header
		{
			MFsize payloadSize{ 0 };//stores size of terrain data - to be done in rle	
			MFu32 xChunks;
			MFu32 zChunks;
			MFu32 nVoxels;
			MFu32 mVoxels;
			MFu32 hVoxels;
			MFu8 worldLOD;
		}header;
		MFint8* payload;//terrain SDF
	};
	size_t Convert_MDB(const MDB_VoxelMap& terrain, Binary_VoxelMap& binaryVoxelMap);
}