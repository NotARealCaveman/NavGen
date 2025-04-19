#pragma once
#include <ManifestParser/OpenGEX/GEX_Metric.h>
#include <ManifestUtility/DebugLogger.h>

#include "MDB/MDB_GameObject.h"
#include "Manifest_DatabaseBuilder.h"

using namespace Manifest_Parser;

namespace Manifest_Persistence
{	
	void BuildResourceDatabase(const DDL_File& file, ManifestResourceDatabaseBuilder& database);

	void BuildWorldDatabase(const DDL_File& file, ManifestWorldDatabaseBuilder& database);
}