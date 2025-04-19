#include "MDB_VoxelMap.h"

using namespace Manifest_Persistence;

ForeignKey Manifest_Persistence::TableEntry(const DDL_Structure& structure, VoxelMapBuildTable& voxelMapBuildTable, const MFu32& xChunks, const MFu32& zChunks, const MFu8& worldLOD)
{
	MDB_VoxelMap& entry = voxelMapBuildTable.entries.emplace_back();
	entry.mapID = voxelMapBuildTable.nextTableIndex++;
	voxelMapBuildTable.mappedEntryKeys.insert({ structure.name.c_str(), entry.mapID });	

	//to be moved to world config type structure
	entry.xChunks = xChunks;
	entry.zChunks = zChunks;
	entry.worldLOD = worldLOD;
	//prepare properties
	const auto& voxelMap = HeapData<MDD_VoxelMap>(structure);
	entry.nVoxels = voxelMap.nVoxels;
	entry.mVoxels = voxelMap.mVoxels;
	entry.hVoxels = voxelMap.hVoxels;	
	//prepare substructures	
	for (const auto& substructure : structure.subSutructres)
	{
		switch (ExtractStructureType(substructure->identifier.c_str()))
		{
			case DDL_BufferTypes::DDL_int8:
			{
				const auto& SDFHeap{ HeapData<DDL_Int8>(*substructure) };
				const auto& samples{ reinterpret_cast<MFint8*>(SDFHeap.data.typeHeap) };
				const auto payloadSize = entry.nVoxels * entry.mVoxels * entry.hVoxels;
				entry.mapSDF = New<MFint8,
					ScratchPad<MFint8>>(payloadSize);
				memcpy(entry.mapSDF, samples, payloadSize);
				break;
			}
			DEFAULT_BREAK
		};
	}

	return entry.mapID;
}