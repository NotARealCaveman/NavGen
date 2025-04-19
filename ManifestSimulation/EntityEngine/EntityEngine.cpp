#include "EntityEngine.h"

using namespace Manifest_Simulation;

EntityEngine::EntityEngine() : entityManager{MAX_ENTITIES}
{

}

MFbool EntityEngine::Init(HivemindSystemEventSpace& hivemindSystemEventSpace, xoshiro256ss_state* _idGenerator)
{
	entityManager.idGenerator = _idGenerator;
	return entityManager.Init(hivemindSystemEventSpace);
}

void EntityEngine::Terminate()
{
	entityManager.entityData.Terminate();
} 

//TODO: WILL NEED TO ADDRESS A WAY FOR REMOVED ENTITIES(AND PHYSICS OBJECTS) TO MESSAGE RENDERER AND REMOVE THEIR RESOURCE INFORMATION
void EntityEngine::UpdateEntities(const MFfloat dt, const MFu32 currentPollingGeneration)
{
	EntityPhysicsData& entityData{ entityManager.entityData };
	//update spawned entities
	for (MFu32 dataIndex{ 0 }; dataIndex < entityData.size(); ++dataIndex)
	{
		//to move into integrate forces function 
		const MFvec3 prevFrameForces{ entityData.GetForce(dataIndex) * entityData.GetIMass(dataIndex) * dt};
		const MFvec3 previousPosition{ entityData.GetPosition(dataIndex) };
		const MFbool isDynamic{ entityData.GetDynamic(dataIndex) };
		const MFbool isGrounded{ entityData.GetGrounded(dataIndex) };
		const MFmat3 entityRotation{ entityData.GetOrientation(dataIndex).GetRotation() };
		const MFfloat updateSpeed = entityData.GetSpeed(dataIndex) * dt;
		const MFpoint3 ds = entityRotation * entityData.GetMovingDirection(dataIndex) * updateSpeed; 
		const MFvec3 newForces{ (ds + (prevFrameForces + (RigidBody_Gravity_Y * !isGrounded) * 1 * dt) * isDynamic) };
		const MFvec3 newPosition{ previousPosition + newForces };
		entityData.SetPreviousPosition(previousPosition, dataIndex);
		entityData.SetPosition(newPosition, dataIndex);
		entityData.SetGrounded(false, dataIndex);		
	};
}

void EntityEngine::IntegrateEntities(const MFfloat dt)
{
	EntityPhysicsData& entityData{ entityManager.entityData };	

	if (!entityData.size())
		return;

	//may as well take advantage of access we shouldn't have
	//damp velocity due to accelaration
	constexpr MFfloat entityDamping{ 0.15 };

	entityData.ClearFrameForces(); 	
	//todo: when entitis go CCD - right now just the current position is simulated
}