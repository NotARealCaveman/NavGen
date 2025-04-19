#pragma once

#include "MDB_Table.h"
#include "MDB_VertexArray.h"
#include "MDB_IndexArray.h"

namespace Manifest_Persistence
{
	
	struct MDB_Mesh
	{
		PrimaryKey meshID = KEY_NOT_PRESENT;		
		VertexArrayIDs vertexArrayIDs;
		ForeignKey indexArrayID;
	}; 
	typedef DatabaseBuildTable<MDB_Mesh> MeshBuildTable;
	ForeignKey TableEntry(const DDL_Structure& structure, MeshBuildTable& meshBuildTable, VertexBuildTables& vertexyBuildTables, IndexBuildTable& indexBuildTable);
}