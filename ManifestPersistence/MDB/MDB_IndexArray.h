#pragma once

#include <ManifestParser/OpenGEX/GEX_Mesh.h>

#include "MDB_Table.h"

using namespace Manifest_Parser;

namespace Manifest_Persistence
{
	struct MDB_IndexArray
	{
		PrimaryKey indexArrayID = KEY_NOT_PRESENT;
		MFu32* indexData = BUFFER_NOT_PRESENT;
		MFsize elements = 0;
	};
	typedef DatabaseBuildTable<MDB_IndexArray> IndexBuildTable;
	ForeignKey TableEntry(const DDL_Structure& structure, IndexBuildTable& indexBuildTable);
}