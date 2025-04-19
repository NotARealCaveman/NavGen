#include "MDB_Terrain.h"

using namespace Manifest_Persistence;

ForeignKey Manifest_Persistence::TableEntry(const DDL_Structure& structure, TerrainBuildTable& terrainBuildTable)
{
	MDB_Terrain& entry = terrainBuildTable.entries.emplace_back();
	entry.terrainID = terrainBuildTable.nextTableIndex++;
	terrainBuildTable.mappedEntryKeys.insert({ structure.name.c_str(), entry.terrainID });
	//prepare properties
	const auto& terrain = HeapData<MDD_Terrain>(structure);
	SetCompositeWard(terrain.xChunkIndex, TERRAIN_Z_INDEX_HASH_OFFSET, entry.terrainIndexHash);
	SetCompositeBow(terrain.zChunkIndex, TERRAIN_Z_INDEX_HASH_OFFSET, entry.terrainIndexHash);
	//prepare substructures
	for (const auto& substructure : structure.subSutructres)
	{
		switch (ExtractStructureType(substructure->identifier.c_str()))
		{
			case DDL_BufferTypes::DDL_uint8:
			{
				const auto& indexHeap{ HeapData<DDL_Uint8>(*substructure) };
				const auto& index{ *reinterpret_cast<MFu8*>(indexHeap.data.typeHeap) };
				entry.terrainID = index;
				break;
			}
			DEFAULT_BREAK
		};
	}
	return entry.terrainID;
}