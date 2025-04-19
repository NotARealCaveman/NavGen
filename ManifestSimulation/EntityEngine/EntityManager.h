#pragma once
#include <ManifestCore/Systems/HivemindSystem.h>
#include <ManifestMath/RNG.h>

#include "Entity.h"
#include "EntitySpawner.h"

using Manifest_Math::xoshiro256ss_state;
using Manifest_Memory::GlobalAllocator;
using Manifest_Core::HivemindSystemEventSpace, Manifest_Core::HivemindSystemEvent, Manifest_Core::HivemindSystemMessageType;

namespace Manifest_Simulation
{
	struct EntityManager	
	{
		EntityManager(const MFsize MAX_ENTITIES);
		MFbool Init(HivemindSystemEventSpace& hivemindSystemEventSpace);
		///Entity Managment///		
		//returns the data index for the entity 
		MFu32 StoreEntity(const EntitySpawnParams& entitySpawnParams, const MFu64 entityID);
		void RemoveEntities(const std::vector<MFu64>& removedIDs);

		///Entitiy Inquiries///
		//returns address of entity ptr resource given an entity ID
		const MFu32 GetEntityIndex(const PrimaryKey entityID) const;
		
				
		EntityPhysicsData entityData;				
		xoshiro256ss_state* idGenerator;	
		HivemindSystemEventSpace* hivemindSystemEventSpace;
	};
};