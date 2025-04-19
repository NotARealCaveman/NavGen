#pragma once

#include <ManifestPersistence/MDB/MDB_GeometryNode.h>

namespace Manifest_Persistence
{	
	struct Binary_GeometryNode
	{
		struct Entry_Header
		{
			size_t	payloadSize{0};
			PrimaryKey nodeID;
			ForeignKey geometryID;
			ForeignKey materialID;			
		}header;
		MFfloat* payload;//represents a transform if present
	};	
	//see Buffer_Types.cpp for details
	size_t Convert_MDB(const MDB_GeometryNode& geometryNode,const ObjectRefBuildTable& objectRefBuildTable, const MaterialRefBuildTable& materialRefBuildTable, Binary_GeometryNode& binaryGeometryNode);
}