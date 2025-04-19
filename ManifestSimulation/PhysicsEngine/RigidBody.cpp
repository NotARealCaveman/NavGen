#include "RigidBody.h"

using namespace Manifest_Simulation;
 
MFbool RigidBodyData::Init()
{ 
	iMass = reinterpret_cast<MFfloat*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFfloat)));	
	linearAcceleration = reinterpret_cast<MFvec3*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFvec3)));
	appliedForce = reinterpret_cast<MFvec3*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFvec3)));
	prevAcceleration = reinterpret_cast<MFvec3*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFvec3)));
	linearVelocity = reinterpret_cast<MFvec3*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFvec3)));
	previousPosition = reinterpret_cast<MFpoint3*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFpoint3)));
	position = reinterpret_cast<MFpoint3*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFpoint3)));
	linearDamping = reinterpret_cast<MFfloat*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFfloat)));
	appliedTorque = reinterpret_cast<MFvec3*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFvec3)));
	angularVelocity = reinterpret_cast<MFvec3*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFvec3)));
	prevOrientation = reinterpret_cast<MFquaternion*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFquaternion)));
	orientation = reinterpret_cast<MFquaternion*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFquaternion)));
	angularDamping = reinterpret_cast<MFfloat*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFfloat)));
	worldSpace = reinterpret_cast<MFtransform*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFtransform)));
	iIntertiaTensorLocal = reinterpret_cast<InverseInertiaTensor*>(calloc(Manifest_Simulation::maxBodies, sizeof(InverseInertiaTensor)));
	iIntertiaTensorWorld = reinterpret_cast<InverseInertiaTensor*>(calloc(Manifest_Simulation::maxBodies, sizeof(InverseInertiaTensor)));
	scale = reinterpret_cast<MFvec3*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFvec3)));
	ID = reinterpret_cast<MFu64*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFu64)));
	isDynamic = reinterpret_cast<MFbool*>(calloc(Manifest_Simulation::maxBodies, sizeof(MFbool)));
	capacity = maxBodies; 

	return true;
}

void RigidBodyData::Terminate()
{	 
	if (iMass) { free(iMass); iMass = nullptr; }
	if (linearAcceleration) { free(linearAcceleration); linearAcceleration = nullptr; }
	if (appliedForce) { free(appliedForce); appliedForce = nullptr; } 
	if (prevAcceleration) { free(prevAcceleration); prevAcceleration = nullptr; }
	if (linearVelocity) { free(linearVelocity); linearVelocity = nullptr; }
	if (previousPosition) { free(previousPosition); previousPosition = nullptr; }
	if (position) { free(position); position = nullptr; }
	if (linearDamping) { free(linearDamping); linearDamping = nullptr; }
	if (appliedTorque) { free(appliedTorque); appliedTorque = nullptr; } 
	if (angularVelocity) { free(angularVelocity); angularVelocity = nullptr; }
	if (prevOrientation) { free(prevOrientation); prevOrientation = nullptr; }
	if (orientation) { free(orientation); orientation = nullptr; } 
	if (angularDamping) { free(angularDamping); angularDamping = nullptr; }
	if (worldSpace) { free(worldSpace); worldSpace = nullptr; }
	if (iIntertiaTensorLocal) { free(iIntertiaTensorLocal); iIntertiaTensorLocal = nullptr; }
	if (iIntertiaTensorWorld) { free(iIntertiaTensorWorld); iIntertiaTensorWorld = nullptr; }
	if (scale) { free(scale); scale = nullptr; }
	if (ID) { free(ID); ID = nullptr; }
	if (isDynamic) { free(isDynamic); isDynamic = nullptr; }
}

void RigidBodyData::resize(const MFsize newSize)
{

}


MFu32 RigidBodyData::AddBody(const RigidBodyParameters& body, const MFu64 physicsID, const MFbool dynamic)
{
	MFu32 result;
	if (freeList.empty())
		result = activeSize++;
	else
	{
		result = freeList.front();
		freeList.pop_front();
	}
	iMass[result] = body.iMass;
	linearAcceleration[result] = body.linearAcceleration;
	appliedForce[result] = body.appliedForce;
	prevAcceleration[result] = body.prevAcceleration;
	linearVelocity[result] = body.linearVelocity;
	previousPosition[result] = body.prevPosition;
	position[result] = body.position;
	linearDamping[result] = body.linearDamping;
	appliedTorque[result] = body.appliedTorque;
	angularVelocity[result] = body.angularVelocity;
	prevOrientation[result] = body.prevOrientation;
	orientation[result] = body.orientation;
	angularDamping[result] = body.angularDamping;
	worldSpace[result] = body.worldSpace;
	iIntertiaTensorLocal[result] = body.iIntertiaTensorLocal;
	iIntertiaTensorWorld[result] = body.iIntertiaTensorWorld;
	ID[result] = physicsID;
	isDynamic[result] = dynamic;
	IDtoIndexMap.insert(std::make_pair(physicsID, result));

	return result;
}

void RigidBodyData::AddBody(const MFu64 physicsID, const MFu32 bodyIndex, const MFbool dynamic)
{
	IDtoIndexMap.insert(std::make_pair(physicsID, bodyIndex));
	isDynamic[bodyIndex] = dynamic;
}

MFu32 RigidBodyData::ReserveSpaceForBodies(const MFsize reservationSize)
{
	MFu32 result{ static_cast<MFu32>(activeSize) };
	activeSize += reservationSize;

	return result;
}

void Manifest_Simulation::ZeroBody(RigidBodyParameters& body)
{		
	body.iMass = 0;
	body.linearAcceleration = 0;
	body.appliedForce = 0;
	body.prevAcceleration= 0;
	body.linearVelocity = 0;
	body.prevPosition = body.position = 0;
	body.linearDamping = 0;
	body.appliedTorque = 0;
	body.angularVelocity = 0;
	body.prevOrientation = body.orientation = { 0,0,0,1 };
	body.angularDamping = 0;
	body.worldSpace = { 0,0,0,0 };	
	body.iIntertiaTensorLocal = { 0,0,0 };
	body.iIntertiaTensorWorld = body.iIntertiaTensorLocal;		
	body.scale = { 1,1,1 };
}

void RigidBodyData::IntegrateForces(const MFdouble& dt)
{
	const MFvec3 gravity{ RigidBody_Gravity_Y };
	if (!activeSize)
		return;
	//calculate prev frame linear forces, f = ma
	std::vector<MFvec3> prevFrameForces;

	const MFu32 size{ static_cast<MFu32>(activeSize) };

	std::transform(appliedForce, appliedForce + size, iMass, std::back_inserter(prevFrameForces), [&](const MFvec3& force, const MFfloat& iMass) {return force * iMass; });
	//calculate and store prev frame accel and add in gravity, ag = f/m + g
	std::transform(linearAcceleration, linearAcceleration + size, &prevFrameForces[0], prevAcceleration, [&](const MFvec3& linearAccelaration, const MFvec3& prevAccelaration) {return linearAccelaration + prevAccelaration + gravity; });
	//calculate new body linear velocities, v += a  * dt
	std::transform(prevAcceleration, prevAcceleration + size, linearVelocity, linearVelocity, [&](const MFvec3& prevAccelaration, const MFvec3& linearVelocity) {  return linearVelocity + (prevAccelaration * dt); });
	//keep linear forces for dynamic bodies only 
	std::transform(linearVelocity, linearVelocity + size, isDynamic, linearVelocity, [&](const MFvec3& linearVelocity, const MFbool isDynamic) {return linearVelocity * isDynamic; });
	//calculate prev frame angular forces	
	std::transform(iIntertiaTensorWorld, iIntertiaTensorWorld + size, appliedTorque, &prevFrameForces[0], [&](const InverseInertiaTensor& tensorW, const MFvec3& torque) {return tensorW * torque; });//prevFroces reused, n linear = n angular writes	
	//calculate new body angular velocities
	std::transform(prevFrameForces.begin(), prevFrameForces.end(), angularVelocity, angularVelocity, [&](const MFvec3& angularAccelaration, const MFvec3& angularVelocity) {return angularVelocity + angularAccelaration * dt; });
	//keep angular forces for dynamic bodies only 
	std::transform(angularVelocity, angularVelocity + size, isDynamic, angularVelocity, [&](const MFvec3& angualrVelocity, const MFbool isDynamic) {return angualrVelocity * isDynamic; });

	//zero out forces for next frame
	memset(appliedForce, 0, sizeof(MFvec3) * activeSize);
	memset(appliedTorque, 0, sizeof(MFvec3) * activeSize);
}

void RigidBodyData::IntegrateVelocities(const MFdouble& dt)
{
	const MFu32 size{ static_cast<MFu32>(activeSize) };
	//linear
	//store old positions
	std::transform(position, position + size, previousPosition, [&](const MFpoint3& prevPosition) {return prevPosition; });
	//calculate new body positions, p = vt (ignore acceleration component)
	std::transform(linearVelocity, linearVelocity + size, position, position, [&](const MFvec3& linearVelocity, const MFpoint3& position) { return position + linearVelocity * dt; });
	//apply damping to linear velocity
	std::transform(linearDamping, linearDamping + size, linearVelocity, linearVelocity, [&](const MFfloat& linearDamping, const MFvec3& linearVelocity) { return linearVelocity * pow(linearDamping, dt); });
	//angular
	//store old orientations
	std::transform(orientation, orientation + size, prevOrientation, [&](const MFquaternion& prevOrientation) {return prevOrientation; });
	//calculate new body orientations
	std::transform(angularVelocity, angularVelocity + size, orientation, orientation, [&](const MFvec3& angularVelocity, const MFquaternion& orientation) { return Normalize(orientation + MFquaternion{ angularVelocity ,0.0 }*orientation * 0.5f * dt); });
	//apply damping to angular velocity
	std::transform(angularDamping, angularDamping + size, angularVelocity, angularVelocity, [&](const MFfloat& angularDamping, const MFvec3& angularVelocity) {return angularVelocity * pow(angularDamping, dt); });
	//calculate current body rotations
	std::vector<MFmat3> bodyRotations;
	std::transform(orientation, orientation + size, std::back_inserter(bodyRotations), [&](const MFquaternion& orientation) {return orientation.GetRotation(); });
	//calculate new world space matrices 
	std::transform(bodyRotations.begin(), bodyRotations.end(), position, worldSpace, [&](const MFmat3& rotation, const MFvec3& position) {return BodyWorldSpace(rotation, position); });
	//calcualte new world intertia matrices
	std::transform(bodyRotations.begin(), bodyRotations.end(), iIntertiaTensorLocal, iIntertiaTensorWorld, [&](const MFmat3& rotation, const InverseInertiaTensor& tensorL) {return InteriaWorldSpace(rotation, tensorL); });
	std::for_each(iIntertiaTensorWorld, iIntertiaTensorWorld + size, [&, bodyIndex = 0](InverseInertiaTensor& tensorW) mutable ->void
	{
		const MFvec3& scale{ this->scale[bodyIndex++] };
		tensorW[0][0] *= scale[0];
		tensorW[1][1] *= scale[1];
		tensorW[2][2] *= scale[2];
	});
}

///GETTERS
const MFvec3& RigidBodyData::GetLinearAccelaration(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return linearAcceleration[dataIndex];
}
const MFvec3& RigidBodyData::GetPreviousAccelaration(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return prevAcceleration[dataIndex];
}
const MFvec3& RigidBodyData::GetLinearVelocity(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return linearVelocity[dataIndex];
} 
const MFfloat& RigidBodyData::GetLinearDamping(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return linearDamping[dataIndex];
}
const MFvec3& RigidBodyData::GetAngularVelocity(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return angularVelocity[dataIndex];
}
const MFquaternion& RigidBodyData::GetPreviousOrientation(const MFu32 dataIndex) const
{
	assert(dataIndex < size());
	
	return prevOrientation[dataIndex];
}
const MFfloat& RigidBodyData::GetAngularDamping(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return angularDamping[dataIndex];
}
const MFtransform& RigidBodyData::GetWorldSpace(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return worldSpace[dataIndex];
}
const InverseInertiaTensor& RigidBodyData::GetLocalITensor(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return iIntertiaTensorLocal[dataIndex];
}
const InverseInertiaTensor& RigidBodyData::GetWorldITensor(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return iIntertiaTensorWorld[dataIndex];
}
const MFbool& RigidBodyData::GetDynamic(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return isDynamic[dataIndex];
}
///SETTERS
void RigidBodyData::SetLinearAccelaration(const MFvec3& linearAcceleration, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->linearAcceleration[dataIndex] = linearAcceleration;
}
void RigidBodyData::SetLinearVelocity(const MFvec3& linearVelocity, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->linearVelocity[dataIndex] = linearVelocity;
}
void RigidBodyData::SetLinearDamping(const MFfloat linearDamping, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->linearDamping[dataIndex] = linearDamping;
}
void RigidBodyData::SetAngularVelocity(const MFvec3& angularVelocity, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->angularVelocity[dataIndex] = angularVelocity;
}
void RigidBodyData::SetAngularDamping(const MFfloat angularDamping, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->angularDamping[dataIndex] = angularDamping;
}
void RigidBodyData::SetWorldSpace(const MFtransform& worldSpace, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->worldSpace[dataIndex] = worldSpace;
}
void RigidBodyData::SetLocalITensor(const InverseInertiaTensor& localITensor, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	iIntertiaTensorLocal[dataIndex] = localITensor;
}
void RigidBodyData::SetWorldITensor(const InverseInertiaTensor& worldITensor, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	iIntertiaTensorWorld[dataIndex] = worldITensor;
}
void RigidBodyData::SetDynamic(const MFbool isDynamic, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->isDynamic[dataIndex] = isDynamic;
}

//converts the bodies orientation and position into a transformation matrix
extern MFtransform Manifest_Simulation::BodyWorldSpace(const MFmat3& worldRotation, const MFpoint3& worldPosition)
{	
	return MFtransform
	{ 
		worldRotation[0],
		worldRotation[1],
		worldRotation[2],
		worldPosition
	};
}
//takes the bodies world space and moves the intertia tensor into it
InverseInertiaTensor Manifest_Simulation::InteriaWorldSpace(const MFmat3& bodyWorldRotation, const InverseInertiaTensor& iTensor)
{	
	return bodyWorldRotation * iTensor* Transpose(bodyWorldRotation) ;
}