#include "MDB_GeometryNode.h"

using namespace Manifest_Persistence;

ForeignKey Manifest_Persistence::TableEntry(const DDL_Structure& structure, const GeometryObjectBuildTable& geometryObjectBuildTable, ObjectRefBuildTable& objectRefBuildTable)
{
	MDB_ObjectRef& entry = objectRefBuildTable.entries.emplace_back();
	entry.objectRefID = objectRefBuildTable.nextTableIndex++;
	objectRefBuildTable.mappedEntryKeys.insert({ structure.name.c_str(),entry.objectRefID });
	const GEX_ObjectRef& ref{ HeapData<GEX_ObjectRef>(structure) };
	entry.numReferences = ref.referenceNames.size();		
	entry.geometryIDs = New<ForeignKey, ScratchPad<ForeignKey>>(entry.numReferences);
	for (auto objectIndex = 0; objectIndex < entry.numReferences; ++objectIndex)
	{
		auto objectRef = geometryObjectBuildTable.mappedEntryKeys.find(ScratchPadString{ static_cast<std::string>(ref.referenceNames[objectIndex]) });
		if (objectRef != geometryObjectBuildTable.mappedEntryKeys.end())
			entry.geometryIDs[objectIndex] = objectRef->second;
		else
			entry.geometryIDs[objectIndex] = KEY_NOT_PRESENT;
	}

	return entry.objectRefID;
}

ForeignKey Manifest_Persistence::TableEntry(const DDL_Structure& structure, const MaterialBuildTable& materialBuildTable, MaterialRefBuildTable& materialRefBuildTable)
{	
	MDB_MaterialRef& entry = materialRefBuildTable.entries.emplace_back();
	entry.materialRefID = materialRefBuildTable.nextTableIndex++;
	materialRefBuildTable.mappedEntryKeys.insert({ structure.name.c_str(),entry.materialRefID });	
	const GEX_MaterialRef& ref{ HeapData<GEX_MaterialRef>(structure) };
	entry.numReferences = ref.referenceNames.size();
	//entry.materialIDs = new ForeignKey[entry.numReferences];
	entry.materialIDs = New<ForeignKey, ScratchPad<ForeignKey>>(entry.numReferences);
	for (auto objectIndex = 0; objectIndex < entry.numReferences; ++objectIndex)
	{
		const auto& view = (ref.referenceNames[objectIndex]);
		auto objectRef = materialBuildTable.mappedEntryKeys.find(ScratchPadString{ static_cast<std::string>(ref.referenceNames[objectIndex]) });
		if (objectRef != materialBuildTable.mappedEntryKeys.end())
			*(entry.materialIDs + objectIndex) = objectRef->second;
		else
			*(entry.materialIDs + objectIndex) = KEY_NOT_PRESENT;
	}

	return entry.materialRefID;
}
ForeignKey Manifest_Persistence::TableEntry(const DDL_Structure& structure,const GeometryObjectBuildTable& geometryObjectBuildTable, const MaterialBuildTable& materialBuildTable, GeometryNodeBuildTable& geometryNodeBuildTable, ObjectRefBuildTable& objectRefBuildTable, MaterialRefBuildTable& materialRefBuildTable)
{
	MDB_GeometryNode& entry = geometryNodeBuildTable.entries.emplace_back();
	entry.nodeID = geometryNodeBuildTable.nextTableIndex++;
	geometryNodeBuildTable.mappedEntryKeys.insert({ structure.name.c_str(), entry.nodeID });
	for (const auto& substructure : structure.subSutructres)
	{
		switch (ExtractStructureType(substructure->identifier.c_str()))
		{
			case GEX_BufferTypes::GEX_ObjectRef:
			{
				entry.objectRefID = TableEntry(*substructure, geometryObjectBuildTable, objectRefBuildTable);
				break;
			}
			case GEX_BufferTypes::GEX_MaterialRef:
				entry.materialRefID = TableEntry(*substructure, materialBuildTable, materialRefBuildTable);
				break;
			case GEX_BufferTypes::GEX_Transform:	
			{
				//entry.transform = new MDB_Transform;				
				entry.transform = New<MDB_Transform,ScratchPad< MDB_Transform>>(1);
				auto transform{ HeapData<GEX_Transform>(*substructure).field.data.typeHeap };
				memcpy(entry.transform->field, transform, sizeof(float) * 16);
				break;
			}
			DEFAULT_BREAK
		}
	}

	return entry.nodeID;
}
