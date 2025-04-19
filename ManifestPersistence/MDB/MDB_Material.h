#pragma once

#include <ManifestParser/OpenGEX/GEX_Material.h>

#include "MDB_Table.h"
#include "MDB_Texture.h"

namespace Manifest_Persistence
{
	struct MDB_Material
	{
		PrimaryKey materialID = KEY_NOT_PRESENT;	
		ForeignKey textureIDs[3]{ KEY_NOT_PRESENT,KEY_NOT_PRESENT,KEY_NOT_PRESENT };//diffuse,normal,parallax
	};
	typedef DatabaseBuildTable<MDB_Material> MaterialBuildTable;
	ForeignKey TableEntry(const DDL_Structure& structure, MaterialBuildTable& materialBuildTable, TextureBuildTable& textureBuildTable);
}