#pragma once
#include <ManifestCommunication/EventSpace.h>
#include <ManifestPersistence/Binary_Database.h>
#include <ManifestUtility/DebugLogger.h>

using namespace Manifest_Persistence;
using namespace Manifest_Communication;

namespace Manifest_Core
{
	struct ObersvableFileSystem
	{		
		enum class MessageTypes : MFu64
		{
			MBD_GEOMETRYNODE = pow2(0),
			MBD_GEOMETRYOBJECT = pow2(1),
			MBD_MATERIAL = pow2(2),
			MBD_TEXTURE = pow2(3),
			MBD_MESH = pow2(4),
			MBD_TERRAIN = pow2(5),
			MBD_VOXELMAP = pow2(6),
			MBD_RIGIDBODY = pow2(7),
			MBD_COLLIDER = pow2(8),
		};					
		static void LoadGameDatabaseFile(const std::string& mbd, EventSpace& eventSpace);
		static void LoadWorldFile(const std::string& mbd, EventSpace& eventSpace);
	};

	using FileSystem = ObersvableFileSystem;
	using FileSystemMessageType = FileSystem::MessageTypes;
	using FileSystemObservationToken = ObservationToken;
	using FileSystemEvent = Event;
	using FileSystemObserver = Observer;
	using FileSystemEventSpace = EventSpace;		
}