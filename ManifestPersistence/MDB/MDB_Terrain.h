#pragma once

#include <ManifestParser/MDD/MDD_Terrain.h>
#include <ManifestParser/DDL_ExtendedTypes.h>

#include "MDB_Table.h"

using namespace Manifest_Parser;

namespace Manifest_Persistence
{
	static constexpr MFu64 TERRAIN_Z_INDEX_HASH_OFFSET{ 32 };
		
	struct MDB_Terrain
	{
		PrimaryKey terrainID;		
		CompositeKey terrainIndexHash;//xIndex(b:0-31), zIndex(b:32-63)
	};
	using TerrainBuildTable = DatabaseBuildTable<MDB_Terrain>;
	ForeignKey TableEntry(const DDL_Structure&  structure, TerrainBuildTable& terrainBuildTable);
}