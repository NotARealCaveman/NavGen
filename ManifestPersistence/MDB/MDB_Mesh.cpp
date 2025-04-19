#include "MDB_Mesh.h"

using namespace Manifest_Persistence;

ForeignKey Manifest_Persistence::TableEntry(const DDL_Structure& structure, MeshBuildTable& meshBuildTable, VertexBuildTables& vertexBuildTables, IndexBuildTable& indexBuildTable)
{	
	MDB_Mesh& entry = meshBuildTable.entries.emplace_back();
	entry.meshID = meshBuildTable.nextTableIndex++;
	meshBuildTable.mappedEntryKeys.insert({structure.name.c_str(), entry.meshID});

	for (const auto& substructure : structure.subSutructres)
	{
		switch (ExtractStructureType(substructure->identifier.c_str()))
		{
			case GEX_BufferTypes::GEX_VertexArray:
			{	
				auto& vaIDs = entry.vertexArrayIDs;
				auto bufferIndex{ HeapData<GEX_VertexArray>(*substructure).bufferIndex };
				if (bufferIndex == 0)
					vaIDs.vertexID = TableEntry(*substructure, vertexBuildTables.vertexTable);
				else if (bufferIndex == 1)
					vaIDs.uvID = TableEntry(*substructure, vertexBuildTables.uvTable);
				else if (bufferIndex == 2)
					vaIDs.normalID = TableEntry(*substructure, vertexBuildTables.normalTable);
				else if (bufferIndex == 3)
					vaIDs.tangentID = TableEntry(*substructure, vertexBuildTables.tangentTable);
				else if (bufferIndex == 4)
					vaIDs.bitangentID = TableEntry(*substructure, vertexBuildTables.bitangentTable);
				break;
			}				
			case GEX_BufferTypes::GEX_IndexArray:
				entry.indexArrayID = TableEntry(*substructure, indexBuildTable);
				break;
			DEFAULT_BREAK
		}
	}
	return entry.meshID;
}