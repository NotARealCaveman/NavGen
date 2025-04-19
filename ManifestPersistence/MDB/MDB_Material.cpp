#include "MDB_Material.h"

using namespace Manifest_Persistence;

ForeignKey Manifest_Persistence::TableEntry(const DDL_Structure& structure, MaterialBuildTable& materialBuildTable, TextureBuildTable& textureBuildTable)
{	
	MDB_Material& entry = materialBuildTable.entries.emplace_back();
	entry.materialID = materialBuildTable.nextTableIndex++;
	materialBuildTable.mappedEntryKeys.insert({ structure.name.c_str(),entry.materialID });
	for (const auto& substructure : structure.subSutructres)
	{
		switch (ExtractStructureType(substructure->identifier.c_str()))
		{
			case GEX_BufferTypes::GEX_Color:
				TableEntry(*substructure, entry.textureIDs, textureBuildTable);
				break;
			case GEX_BufferTypes::GEX_Texture://gex-textures tbd
				TableEntry(*substructure, entry.textureIDs, textureBuildTable);
				break;	
			DEFAULT_BREAK
		}
	}
	return entry.materialID;
}