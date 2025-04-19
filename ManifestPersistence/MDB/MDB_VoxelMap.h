#pragma once

#include <ManifestParser/MDD/MDD_VoxelMap.h>
#include <ManifestParser/DDL_ExtendedTypes.h>

#include "MDB_Table.h"

using namespace Manifest_Parser;

namespace Manifest_Persistence
{	
	struct MDB_VoxelMap
	{
		PrimaryKey mapID;
		MFint8* mapSDF;			
		MFu32 nVoxels;
		MFu32 mVoxels;
		MFu32 hVoxels;
		//to be moved into a world config type structure
		MFu32 xChunks;
		MFu32 zChunks;
		MFu8 worldLOD;
	};
	using VoxelMapBuildTable = DatabaseBuildTable<MDB_VoxelMap>;
	ForeignKey TableEntry(const DDL_Structure& structure, VoxelMapBuildTable& voxelMapBuildTable, const MFu32& xChunks, const MFu32& zChunks, const MFu8& worldLOD);
}