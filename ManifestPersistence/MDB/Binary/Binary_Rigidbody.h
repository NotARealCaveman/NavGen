#pragma once

#include <ManifestPersistence/MDB/MDB_RigidBody.h>

namespace Manifest_Persistence
{
	struct Binary_RigidBody
	{
		struct Entry_Header
		{
			size_t	payloadSize{ 0 };	
			MFsize bodyCount;
			MFsize positionOffset;
			MFsize linearAccelarationOffset;
			MFsize linearVelocityOffset;
			MFsize angularVelocityOffset;
			MFsize scaleOffset;
			MFsize appliedForceOffset;
			MFsize appliedTorqueOffset;
			MFsize iMassOffset;
			MFsize linearDampingOffset;
			MFsize angularDampingOffset;
			MFsize objectIDOffset;
			MFsize dynamicOffset;
		}header;		
		Byte* payload;//represents a transform if present
	};
	//see Buffer_Types.cpp for details
	size_t Convert_MDB(const MDB_Rigidbody& rigidBody, Binary_RigidBody& binaryRigidBody);
}