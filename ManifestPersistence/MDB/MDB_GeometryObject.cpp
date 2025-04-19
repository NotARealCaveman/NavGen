#include "MDB_GeometryObject.h"

using namespace Manifest_Persistence;

ForeignKey Manifest_Persistence::TableEntry(const DDL_Structure& structure, GeometryObjectBuildTable& geometryObjectBuildTable, MeshBuildTable& meshBuildTable, VertexBuildTables& vertexBuildTables, IndexBuildTable& indexBuildTable)
{		
	MDB_GeometryObject& entry = geometryObjectBuildTable.entries.emplace_back();	
	entry.geometryID = geometryObjectBuildTable.nextTableIndex++;
	geometryObjectBuildTable.mappedEntryKeys.insert({ structure.name.c_str(),entry.geometryID });
	
	for (const auto& substructure : structure.subSutructres)
	{
		switch (ExtractStructureType(substructure->identifier.c_str()))
		{
			case GEX_BufferTypes::GEX_Mesh:
				entry.meshID = TableEntry(*substructure, meshBuildTable, vertexBuildTables, indexBuildTable);
				break;
			DEFAULT_BREAK
		}
	}	
	return entry.geometryID;
}