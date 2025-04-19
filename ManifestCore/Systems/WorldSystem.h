#pragma once

#include <ManifestCommunication/EventSpace.h>
#include <ManifestPersistence/Binary_Database.h>
#include <ManifestUtility/DebugLogger.h>

using namespace Manifest_Persistence;
using namespace Manifest_Communication;

namespace Manifest_Core
{
	struct ObersvableWorldSystem
	{
		enum class MessageTypes : MFu64
		{
			NEW_WORLD_MAP = pow2(0),
			NEW_VOXEL_SDF = pow2(1)			
		};		
		static void NotifyNewWorldMap(const MFsize mapChunks, EventSpace& eventSpace);
		static void NotifyNewVoxelSDF(std::vector<MFu32>& indexUpdates, EventSpace& eventSpace);
	};
	using WorldSystem = ObersvableWorldSystem;
	using WorldSystemMessageType = WorldSystem::MessageTypes;
	using WorldSystemObservationToken = ObservationToken;
	using WorldSystemEvent = Event;
	using WorldSystemObserver = Observer;
	using WorldSystemEventSpace = EventSpace;
}