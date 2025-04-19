#pragma once

#include <ManifestParser/OpenGEX/GEX_GeometryObject.h>

#include "MDB_Table.h"
#include "MDB_Mesh.h" 

using namespace Manifest_Parser;

namespace Manifest_Persistence
{
	struct MDB_GeometryObject
	{
		PrimaryKey geometryID = KEY_NOT_PRESENT;
		ForeignKey meshID = KEY_NOT_PRESENT;
		ForeignKey morphID = KEY_NOT_PRESENT;
	};
	typedef DatabaseBuildTable<MDB_GeometryObject> GeometryObjectBuildTable;
	ForeignKey TableEntry(const DDL_Structure& structure, GeometryObjectBuildTable& geometryObjectBuildTable, MeshBuildTable& meshBuildTable, VertexBuildTables& vertexBuildTables, IndexBuildTable& indexBuildTable);
	
}