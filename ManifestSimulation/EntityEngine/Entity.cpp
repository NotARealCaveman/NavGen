#include "Entity.h"

using namespace Manifest_Simulation;
 
//ENTITIES ARE ONLY ALLOWED TO ROTATE ALONG THE Y AXIS
const InverseInertiaTensor EntityPhysicsData::ENTITY_TENSOR
{
	0.0f,0.0f,0.0f,
	0.0f,0.0f,0.0f,
	0.0f,0.0f,0.0f
};

MFbool EntityPhysicsData::Init()
{	
	ID = reinterpret_cast<PrimaryKey*>(calloc(MAX_ENTITIES,sizeof(PrimaryKey)));
	orientation = reinterpret_cast<MFquaternion*>(calloc(MAX_ENTITIES,sizeof(MFquaternion)));
	position = reinterpret_cast<MFpoint3*>(calloc(MAX_ENTITIES,sizeof(MFpoint3)));
	previousPosition = reinterpret_cast<MFpoint3*>(calloc(MAX_ENTITIES, sizeof(MFpoint3)));
	movingDirection = reinterpret_cast<MFvec3*>(calloc(MAX_ENTITIES, sizeof(MFvec3)));
	appliedForce = reinterpret_cast<MFvec3*>(calloc(MAX_ENTITIES,sizeof(MFvec3)));
	appliedTorque = reinterpret_cast<MFvec3*>(calloc(MAX_ENTITIES, sizeof(MFvec3)));
	scale = reinterpret_cast<MFvec3*>(malloc(MAX_ENTITIES * sizeof(MFvec3)));
	iMass = reinterpret_cast<MFfloat*>(calloc(MAX_ENTITIES,sizeof(MFfloat)));
	speed = reinterpret_cast<MFfloat*>(calloc(MAX_ENTITIES, sizeof(MFfloat)));
	isDynamic = reinterpret_cast<MFbool*>(calloc(MAX_ENTITIES, sizeof(MFbool)));
	isGrounded = reinterpret_cast<MFbool*>(calloc(MAX_ENTITIES, sizeof(MFbool)));

	if (scale)
		std::for_each(scale, &scale[MAX_ENTITIES], [&](MFvec3& scale) {scale = { 1 }; });
	if(isDynamic)
		memset(isDynamic, true, MAX_ENTITIES * sizeof(MFbool));
	if(isGrounded)
		memset(isGrounded, true, MAX_ENTITIES * sizeof(MFbool));	
	
	return true;
}

void EntityPhysicsData::Terminate()
{
	if (ID) { free(ID); ID = nullptr; }
	if (orientation) { free(orientation); orientation = nullptr; }
	if (position) { free(position); position = nullptr; }
	if (movingDirection) { free(movingDirection); movingDirection = nullptr; }
	if (appliedForce) { free(appliedForce); appliedForce = nullptr; }
	if (iMass) { free(iMass); iMass = nullptr; }
	if (speed) { free(speed); speed = nullptr; }
	if (scale) { free(scale); scale = nullptr; }	
	if (isDynamic) { free(isDynamic); isDynamic = nullptr; }
	if (isGrounded) { free(isGrounded); isDynamic = nullptr; }
}

void EntityPhysicsData::resize(const MFsize newSize) 
{

}
 

MFu32 EntityPhysicsData::AddEntity(const EntitySpawnParams& entitySpawnParams, const MFu64 entityID)
{
	MFu32 result{ static_cast<MFu32>(activeSize++) };
	ID[result] = entityID;
	orientation[result] = entitySpawnParams.orientation;
	position[result] = entitySpawnParams.position;
	appliedForce[result] = entitySpawnParams.appliedForce;
	appliedTorque[result] = entitySpawnParams.appliedTorque;
	scale[result] = entitySpawnParams.scale;
	iMass[result] = entitySpawnParams.iMass;
	speed[result] = entitySpawnParams.speed;
	isDynamic[result] = entitySpawnParams.dynamic;	

	IDtoIndexMap.insert(std::make_pair(entityID, result));

	return result;
}

///GETTERS
const MFvec3& EntityPhysicsData::GetMovingDirection(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return movingDirection[dataIndex];
}
const MFfloat EntityPhysicsData::GetSpeed(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return this->speed[dataIndex];
}
const MFbool EntityPhysicsData::GetDynamic(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return isDynamic[dataIndex];
}
const MFbool EntityPhysicsData::GetGrounded(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return isGrounded[dataIndex];
}
///SETTERS
void EntityPhysicsData::SetMovingDirection(const MFvec3& movingDirection, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->movingDirection[dataIndex] = movingDirection;
} 
void EntityPhysicsData::SetSpeed(const MFfloat speed, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->speed[dataIndex] = speed;
}
void EntityPhysicsData::SetDynamic(const MFbool isDynamic, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->isDynamic[dataIndex] = isDynamic;
}

void EntityPhysicsData::SetGrounded(const MFbool isGrounded, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->isGrounded[dataIndex] = isGrounded;
}


void EntityPhysicsData::ClearFrameForces()
{
	//pmr integration will "wink" this memory
	memset(appliedForce, 0, sizeof(MFvec3) * activeSize);
	memset(appliedTorque, 0, sizeof(MFvec3) * activeSize);
	memset(movingDirection, 0, sizeof(MFvec3) * activeSize);
}