#include "PhysicsEngine.h"

using namespace Manifest_Simulation;


MFbool PhysicsEngine::Init(xoshiro256ss_state* _idGenerator)
{
	idGenerator = _idGenerator;
	if (!rigidBodies.Init())
		return false;

	return true;
}

void PhysicsEngine::Terminate()
{
	rigidBodies.Terminate();
}
 
void PhysicsEngine::SetBodyDynamics(const MFu32& bodyIndex, const MFbool dynamic)
{
	rigidBodies.SetDynamic(dynamic,bodyIndex); 
}

MFbool PhysicsEngine::IsBodyMapped(const PrimaryKey& physicsID) const
{	 
	return rigidBodies.GetIDIndex(physicsID) != DataSystem::ID_NOT_MAPPED;
}

MFbool PhysicsEngine::IsBodyDynamic(const PrimaryKey& physicsID) const
{
	const MFu32 bodyIndex{ rigidBodies.GetIDIndex(physicsID) };
	assert(bodyIndex != DataSystem::ID_NOT_MAPPED);	

	return rigidBodies.GetDynamic(bodyIndex);
}

MFu32 PhysicsEngine::GetBodyIndex(const PrimaryKey& physicsID) const
{
	const MFu32 bodyIndex{ rigidBodies.GetIDIndex(physicsID) };
	assert(bodyIndex != DataSystem::ID_NOT_MAPPED);

	return bodyIndex; 
}

void PhysicsEngine::IntegratePhysicsForces(const MFfloat dt)
{
	rigidBodies.IntegrateForces(dt);
}

void PhysicsEngine::IntegratePhysicsVelocities(const MFfloat dt)
{
	rigidBodies.IntegrateVelocities(dt);
}