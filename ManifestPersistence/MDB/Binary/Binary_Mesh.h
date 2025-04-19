#pragma once

#include <ManifestPersistence/MDB/MDB_Mesh.h>

namespace Manifest_Persistence
{
	struct Binary_Mesh
	{
		struct Entry_Header
		{
			MFsize payloadSize{ 0 };//stores vbo+ebo size in bytes
			PrimaryKey meshID;//stored database id of the entry
			MFsize eboOffset{ 0 };//stores the byte offset from the beginning of the payload to the index data. interleaved float data is stored immediately followed by the integer index data
			MFsize vboStride{ 0 };
			MFu8 activeArrayAttributes{ 0 };//8 bit code, bits 0-4 are p,uv,n,t,b 5-7 are unused
		}header;		
		float* payload;
	};
	//see Buffer_Types.cpp for details
	size_t Convert_MDB(const MDB_Mesh& mesh, const VertexBuildTables& vertexBuildTables, const IndexBuildTable& indexBuildTable, Binary_Mesh& binaryMesh);
}