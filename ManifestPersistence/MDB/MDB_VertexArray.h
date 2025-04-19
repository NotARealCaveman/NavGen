#pragma once

#include <ManifestParser/OpenGEX/GEX_Mesh.h>

#include "MDB_Table.h"

using namespace Manifest_Parser;

namespace Manifest_Persistence
{
	struct MDB_VertexArray
	{
		PrimaryKey vertexArrayID =	KEY_NOT_PRESENT;
		float* vertexData = BUFFER_NOT_PRESENT;				
		size_t elements = 0;
	};

	typedef DatabaseBuildTable<MDB_VertexArray> VertexArrayBuildTable;
	ForeignKey TableEntry(const DDL_Structure& structure, VertexArrayBuildTable& vertexArrayBuildTable);
	typedef VertexArrayBuildTable VertexTable;
	typedef VertexArrayBuildTable UVTable;
	typedef VertexArrayBuildTable NormalTable;
	typedef VertexArrayBuildTable TangentTable;
	typedef VertexArrayBuildTable BitangentTable;

	struct VertexBuildTables
	{
		VertexTable vertexTable;
		UVTable uvTable;
		NormalTable normalTable;
		TangentTable tangentTable;
		BitangentTable bitangentTable;;
	};
	struct VertexArrayIDs
	{
		ForeignKey vertexID = KEY_NOT_PRESENT;
		ForeignKey uvID = KEY_NOT_PRESENT;
		ForeignKey normalID = KEY_NOT_PRESENT;
		ForeignKey tangentID = KEY_NOT_PRESENT;
		ForeignKey bitangentID = KEY_NOT_PRESENT;;
	};
}