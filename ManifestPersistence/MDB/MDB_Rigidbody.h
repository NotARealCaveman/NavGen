#pragma once
#include <ManifestGLFramework/ManifestMath/Quaternion.h>
#include <ManifestGLFramework/ManifestMath/Point3.h>

#include <ManifestParser/MDD/MDD_PhysicsNode.h>
#include <ManifestParser/MDD/MDD_GameObject.h>
#include <ManifestParser/DDL_ExtendedTypes.h>


#include "MDB_Table.h"

using namespace Manifest_Parser;
using namespace Manifest_Math;

namespace Manifest_Persistence
{	
	struct MDB_Rigidbody
	{		
		PrimaryKey rigidBodyID;
		MFsize bodyCount{0};
		MFquaternion* orientation;
		MFpoint3* position;
		MFvec3* linearAccelaration;
		MFvec3* linearVelocity;
		MFvec3* angularVelocity;
		MFvec3* scale;
		MFvec3* appliedForce;
		MFvec3* appliedTorque;
		MFfloat* iMass;
		MFfloat* linearDamping;
		MFfloat* angularDamping;
		MFu64* objectID;
		MFbool* dynamic;
	};
	using RigidbodyBuildTable = DatabaseBuildTable<MDB_Rigidbody>;
	//converts from AOS to SOA type data
	ForeignKey TableEntry(const ScratchPadVector<DDL_Structure*>& gameObjectStructures,const ScratchPadVector<DDL_Structure*>& physicsNodesStructures, RigidbodyBuildTable& rigidbodyBuildTable);
}