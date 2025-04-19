#include "MDB_Collider.h"

using namespace Manifest_Persistence;

ForeignKey Manifest_Persistence::TableEntry(const ScratchPadVector<DDL_Structure*>& gameObjectStructures, const DDL_Structure* physicsNodesStructure, ColliderBuildTable& colliderBuildTable)
{
	MDB_Collider& entry = colliderBuildTable.entries.emplace_back();
	const auto& physicsNode{ HeapData<MDD_PhysicsNode>(*physicsNodesStructure) };
	entry.dynamic = physicsNode.isDynamic;
	entry.colliderID = colliderBuildTable.nextTableIndex++;
	colliderBuildTable.mappedEntryKeys.insert({ "RigidBody" + entry.colliderID , entry.colliderID });	
	const auto& collider{ physicsNode.collider };	
	entry.colliderType = collider.colliderType;
	const auto& data{ collider.colliderData.data };
	entry.colliderDataElements=  data.subBufferCount * data.subBufferElements ;
	entry.colliderData = New<MFfloat>(entry.colliderDataElements);
	memcpy(entry.colliderData, data.typeHeap, sizeof(MFfloat) * entry.colliderDataElements);
	for (const auto& gameObject : gameObjectStructures)
	{
		const auto& object{ HeapData<MDD_GameObject>(*gameObject) };
		//physics references are first in the list
		const auto& ref{ object.objectReferences.referenceNames[0] };		
		if (ref == physicsNodesStructure->name)
		{
			entry.objectID = *reinterpret_cast<MFu64*>(object.objectID.data.typeHeap);
			break;
		}
	}

	return entry.colliderID;
}