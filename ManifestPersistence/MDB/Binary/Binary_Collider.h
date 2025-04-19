#pragma once

#include <ManifestPersistence/MDB/MDB_Collider.h>

namespace Manifest_Persistence
{
	struct Binary_Collider
	{
		struct Entry_Header
		{
			size_t	payloadSize{ 0 };
			MFu64 objectID;
			MFu8 colliderType;		
			MFbool dynamic;
		}header;
		MFfloat* payload;
	};
	//see Buffer_Types.cpp for details
	size_t Convert_MDB(const MDB_Collider& collider, Binary_Collider& binaryCollider);
}