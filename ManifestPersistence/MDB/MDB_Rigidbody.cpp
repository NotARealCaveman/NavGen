#include "MDB_Rigidbody.h"

using namespace Manifest_Persistence;

ForeignKey Manifest_Persistence::TableEntry(const ScratchPadVector<DDL_Structure*>& gameObjectStructures, const ScratchPadVector<DDL_Structure*>& physicsNodesStructures, RigidbodyBuildTable& rigidbodyBuildTable)
{
	MDB_Rigidbody& entry = rigidbodyBuildTable.entries.emplace_back();	
	entry.rigidBodyID = rigidbodyBuildTable.nextTableIndex++;
	rigidbodyBuildTable.mappedEntryKeys.insert({ "RigidBody"+entry.rigidBodyID, entry.rigidBodyID });
	const auto bodyCount{ physicsNodesStructures.size() };	
	entry.orientation = New<MFquaternion>(bodyCount);
	entry.position = New<MFpoint3>(bodyCount);
	entry.linearAccelaration = New<MFvec3>(bodyCount);
	entry.linearVelocity = New<MFvec3>(bodyCount);
	entry.angularVelocity = New<MFvec3>(bodyCount);
	entry.scale = New<MFvec3>(bodyCount);
	entry.appliedForce = New<MFvec3>(bodyCount);
	entry.appliedTorque = New<MFvec3>(bodyCount);
	entry.iMass = New<MFfloat>(bodyCount);
	entry.linearDamping = New<MFfloat>(bodyCount);
	entry.angularDamping = New<MFfloat>(bodyCount);
	entry.objectID = New<MFu64>(bodyCount);
	entry.dynamic = New<MFbool>(bodyCount);
	auto& bodyIndex{ entry.bodyCount };
	for (const auto& structure : physicsNodesStructures)
	{			
		const auto& physicsNode{ HeapData<MDD_PhysicsNode>(*structure) };	
		const auto& rigidBodyParams{ physicsNode.rigidBodyParams };			
		DLOG({ CONSOLE_GREEN }, "Orientation read at: " , &rigidBodyParams.orientation.data.typeHeap);
		entry.orientation[bodyIndex] = *reinterpret_cast<const MFquaternion*>(rigidBodyParams.orientation.data.typeHeap);
		entry.position[bodyIndex] = *reinterpret_cast<const MFpoint3*>(rigidBodyParams.position.data.typeHeap);
		entry.linearAccelaration[bodyIndex] = *reinterpret_cast<const MFvec3*>(rigidBodyParams.linearAccelaration.data.typeHeap);
		entry.linearVelocity[bodyIndex] = *reinterpret_cast<const MFvec3*>(rigidBodyParams.linearVelocity.data.typeHeap);
		entry.angularVelocity[bodyIndex] = *reinterpret_cast<const MFvec3*>(rigidBodyParams.angularVelocity.data.typeHeap);
		entry.scale[bodyIndex] = *reinterpret_cast<const MFvec3*>(rigidBodyParams.scale.data.typeHeap);
		entry.appliedForce[bodyIndex] = *reinterpret_cast<const MFvec3*>(rigidBodyParams.appliedForce.data.typeHeap);
		entry.appliedTorque[bodyIndex] = *reinterpret_cast<const MFvec3*>(rigidBodyParams.appliedTorque.data.typeHeap);
		entry.iMass[bodyIndex] = *reinterpret_cast<const MFfloat*>(rigidBodyParams.iMass.data.typeHeap);
		entry.linearDamping[bodyIndex] = *reinterpret_cast<const MFfloat*>(rigidBodyParams.linearDamping.data.typeHeap);
		entry.angularDamping[bodyIndex] = *reinterpret_cast<const MFfloat*>(rigidBodyParams.angularDamping.data.typeHeap);
		entry.dynamic[bodyIndex] = physicsNode.isDynamic;
		for (const auto& gameObject : gameObjectStructures)
		{
			const auto& object{ HeapData<MDD_GameObject>(*gameObject) };
			//physics references are first in the list
			const auto& ref{ object.objectReferences.referenceNames[0] };
			if (ref == structure->name)
			{
				entry.objectID[bodyIndex] = *reinterpret_cast<MFu64*>(object.objectID.data.typeHeap);
				break;
			}
		}
		++bodyIndex;
	};
	return 1;
}