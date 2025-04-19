#pragma once

#include "MDB_Table.h"
#include "MDB_Material.h"
#include "MDB_GeometryObject.h"


using namespace Manifest_Parser;

namespace Manifest_Persistence
{
	struct MDB_ObjectRef
	{
		PrimaryKey objectRefID = KEY_NOT_PRESENT;
		ForeignKey* geometryIDs = BUFFER_NOT_PRESENT;
		size_t numReferences = 0;
	};
	typedef DatabaseBuildTable<MDB_ObjectRef> ObjectRefBuildTable;
	ForeignKey TableEntry(const DDL_Structure& structure,const GeometryObjectBuildTable& geometryObjectBuildTable, ObjectRefBuildTable& objectRefBuildTable);

	struct MDB_MaterialRef
	{
		PrimaryKey materialRefID = KEY_NOT_PRESENT;
		ForeignKey* materialIDs = BUFFER_NOT_PRESENT;
		size_t numReferences = 0;
	};
	typedef DatabaseBuildTable<MDB_MaterialRef> MaterialRefBuildTable;
	ForeignKey TableEntry(const DDL_Structure& structure, const MaterialBuildTable& materialBuildTable, MaterialRefBuildTable& materialRefBuildTable);
	
	//every transform will end up being 16 characters - this acts as a conversion type between gex transforms to mdb transforms
	struct MDB_Transform { float field[TransformSize]; };

	struct MDB_GeometryNode
	{
		PrimaryKey nodeID = KEY_NOT_PRESENT;
		ForeignKey objectRefID = KEY_NOT_PRESENT;
		ForeignKey materialRefID = KEY_NOT_PRESENT;		
		MDB_Transform* transform = BUFFER_NOT_PRESENT;		
		//const uint32_t morphWeight;
		//used in transfering game object references per node to build
		ScratchPadVector<PrimaryKey> gameObjectReferences;
	};
	typedef DatabaseBuildTable<MDB_GeometryNode> GeometryNodeBuildTable;
	ForeignKey TableEntry(const DDL_Structure& structure,const GeometryObjectBuildTable& geometryObjectBuildTable, const MaterialBuildTable& materialBuildTable, GeometryNodeBuildTable& geometryNodeBuildTable, ObjectRefBuildTable& objectRefBuildTable, MaterialRefBuildTable& materialRefBuildTable);

	struct Manifest_GeometryNode
	{
		PrimaryKey manifestNodeID;//id imported form the binary database
	};
}