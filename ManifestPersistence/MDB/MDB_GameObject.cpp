#include "MDB_GameObject.h"

using namespace Manifest_Persistence;

void Manifest_Persistence::TableEntry(const ScratchPadVector<DDL_Structure*>& gameObjects, const ScratchPadVector<DDL_Structure*>& geometryNodes, const GeometryObjectBuildTable& geometryObjectTable, const MaterialBuildTable& materialBuildTable, GeometryNodeBuildTable& geometryNodeBuildTable, ObjectRefBuildTable& objectRefBuildTable, MaterialRefBuildTable& materialRefBuildTable)
{
	std::unordered_map<ScratchPadString,PrimaryKey> encounteredNodes;	
	for (const auto& object : gameObjects)
	{				
		const auto& gameObject{ HeapData<MDD_GameObject>(*object) };
		//in the future will need to supply a way to determine bit offset in reference list with object type
		const auto& referenceName{ gameObject.objectReferences.referenceNames[1] };
		for (const auto& node: geometryNodes)
		{			
			const auto& nodeName{ node->name };
			//find geometry node this object uses
			if (referenceName == nodeName)
			{
				//check if first encounter
				const auto encounteredNode = encounteredNodes.find(nodeName);
				if (encounteredNode == encounteredNodes.end())
				{
					//insert node into table
					const auto nodeIndex = TableEntry(*node, geometryObjectTable, materialBuildTable, geometryNodeBuildTable, objectRefBuildTable, materialRefBuildTable);
					//add inital object reference
					auto& geoemtryNode{ geometryNodeBuildTable.entries[nodeIndex] };
					geoemtryNode.gameObjectReferences.emplace_back(*reinterpret_cast<MFu64*>(gameObject.objectID.data.typeHeap));
					//insert node into map
					encounteredNodes.insert({ nodeName,nodeIndex });
					break;
				}
				else
				{
					//look up node reference by id 
					auto& geoemtryNode{ geometryNodeBuildTable.entries[encounteredNode->second] };
					//add additional object reference
					geoemtryNode.gameObjectReferences.emplace_back(*reinterpret_cast<MFu64*>(gameObject.objectID.data.typeHeap));
				}
			}
		}		
	} 
}

void Manifest_Persistence::TableEntry(const ScratchPadVector<DDL_Structure*>& gameObjects, const ScratchPadVector<DDL_Structure*>& physicsNodes)
{

}