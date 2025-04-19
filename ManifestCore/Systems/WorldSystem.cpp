#include "WorldSystem.h"

using namespace Manifest_Core;


void Manifest_Core::ObersvableWorldSystem::NotifyNewWorldMap(const MFsize mapChunks, EventSpace& eventSpace)
{
	WorldSystemEvent wsEvent;
	//create new world map message
	wsEvent.eventToken |= UnderlyingType(MessageTypes::NEW_WORLD_MAP);	
	wsEvent.messages.emplace_back(Message{ UnderlyingType(MessageTypes::NEW_WORLD_MAP), mapChunks });
	//create updated chunks message - all chunks are updated with the new map so all are false for copy status
	std::vector<MFu32> chunkIndexCopyStatus(mapChunks, 0);
	wsEvent.eventToken |= UnderlyingType(MessageTypes::NEW_VOXEL_SDF);
	wsEvent.messages.emplace_back(Message{ UnderlyingType(MessageTypes::NEW_VOXEL_SDF), std::move(chunkIndexCopyStatus) });

	eventSpace.NotifyRegisteredObservers(std::move(wsEvent));	
}

void ObersvableWorldSystem::NotifyNewVoxelSDF(std::vector<MFu32>& indexUpdates, EventSpace& eventSpace)
{
	WorldSystemEvent wsEvent;
	wsEvent.eventToken |= UnderlyingType(MessageTypes::NEW_VOXEL_SDF);
	//no contents in this message, purely used for signaling
	wsEvent.messages.emplace_back(Message{ UnderlyingType(MessageTypes::NEW_VOXEL_SDF), std::move(indexUpdates) });

	eventSpace.NotifyRegisteredObservers(std::move(wsEvent));
}