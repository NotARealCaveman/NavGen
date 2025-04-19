#pragma once

#include <ManifestParser/MDD/MDD_GameObject.h>
#include <ManifestParser/MDD/MDD_PhysicsNode.h>
#include <ManifestParser/DDL_ExtendedTypes.h>

#include "MDB_Table.h"

using namespace Manifest_Parser;

namespace Manifest_Persistence
{
	struct MDB_Collider
	{		
		PrimaryKey colliderID;		
		MFu64 objectID;				
		MFu8 colliderType;		
		MFfloat* colliderData;		
		MFsize colliderDataElements;
		MFbool dynamic;
	};
	using ColliderBuildTable = DatabaseBuildTable<MDB_Collider>;
	ForeignKey TableEntry(const ScratchPadVector<DDL_Structure*>& gameObjectStructures, const DDL_Structure* physicsNodesStructure, ColliderBuildTable& colliderBuildTable);
}