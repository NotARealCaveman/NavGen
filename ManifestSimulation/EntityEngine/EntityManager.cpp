#include "EntityManager.h"

using namespace Manifest_Simulation;

EntityManager::EntityManager(const MFsize MAX_ENTITIES) 
{
}

MFbool EntityManager::Init(HivemindSystemEventSpace& hivemindSystemEventSpace)
{	
	this->hivemindSystemEventSpace = &hivemindSystemEventSpace;

	return entityData.Init();
}

MFu32 EntityManager::StoreEntity(const EntitySpawnParams& entitySpawnParams, const MFu64 entityID)
{
	return entityData.AddEntity(entitySpawnParams, entityID);
}


void EntityManager::RemoveEntities(const std::vector<MFu64>& removedIDs)
{		
	HivemindSystemEvent hivemindEvent;
	hivemindEvent.eventToken = UnderlyingType(HivemindSystemMessageType::OBJECT_REGISTRATION);
	Manifest_Core::ObjectRegistrationMessage removedAgentDetails;
	removedAgentDetails.addObject = false;
	//despawn removed entities
	std::ranges::for_each(removedIDs, [&hivemindEvent, &removedAgentDetails,this](const MFu64 removedID)->void
	{		
		const MFu32 entityDataIndex{ entityData.GetIDIndex(removedID) };
		assert(entityDataIndex != DataSystem::ID_NOT_MAPPED);
		//update IDs for state table upload - will be sending over blank data(processed only, never rendered)				
		entityData.SetID(DataSystem::ID_NOT_MAPPED, entityDataIndex);
		//capture removal information for hivemind message	
		removedAgentDetails.objectID = removedID;
		Message combatRegistrationMessage{ UnderlyingType(HivemindSystemMessageType::OBJECT_REGISTRATION), removedAgentDetails };
		hivemindEvent.messages.emplace_back(std::move(combatRegistrationMessage));
	});
	//send removed entities to hivemind to unregister any agents
	hivemindSystemEventSpace->NotifyRegisteredObservers(std::move(hivemindEvent));
	//remove entities from mappings
	entityData.erase(removedIDs);
}

const MFu32 EntityManager::GetEntityIndex(const PrimaryKey entityID) const
{
	return entityData.GetIDIndex(entityID);
}