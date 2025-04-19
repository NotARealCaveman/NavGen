#pragma once
#include <unordered_map>

#include <ManifestParser/MDD/MDD_GameObject.h>
#include "MDB_GeometryNode.h"

using namespace Manifest_Parser;

namespace Manifest_Persistence
{	 
	//Table entry for Game objects with a graphics component
	void TableEntry(const ScratchPadVector<DDL_Structure*>& gameObjects, const ScratchPadVector<DDL_Structure*>& geometryNodes, const GeometryObjectBuildTable& geometryObjectTable, const MaterialBuildTable& materialBuildTable, GeometryNodeBuildTable& geometryNodeBuildTable, ObjectRefBuildTable& objectRefBuildTable, MaterialRefBuildTable& materialRefBuildTable);
	//Table entry for game objects with a phyiscs component
	void TableEntry(const ScratchPadVector<DDL_Structure*>& gameObjects, const ScratchPadVector<DDL_Structure*>& physicsNodes);
}