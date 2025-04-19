#pragma once

#include <ManifestParser/MDD/MDD_PhysicsNode.h>
#include <ManifestParser/DDL_ExtendedTypes.h>

#include "MDB_Table.h"

using namespace Manifest_Parser;

//to be repurposed to represent a physics node chunk in the world
namespace Manifest_Persistence
{
	struct MDB_PhysicsNode
	{
		PrimaryKey physicsNodeID;
		ForeignKey rigidBodyID;
		ForeignKey colliderID;
		MFbool dynamic;
	};
	using PhysicsNodeBuildTable = DatabaseBuildTable<MDB_PhysicsNode>;	
}