#pragma once

#include <ManifestSimulation/DataSystem.h>
#include <ManifestUtility/DebugLogger.h>

#include <ManifestSimulation/CollisionEngine/Colliders/Tensors.h>;

using namespace Manifest_Math;

namespace Manifest_Simulation
{
	const auto RigidBody_Gravity_X = MFvec3{ -9.8,0.0,0.0 }*1.0f;
	const auto RigidBody_Gravity_Y = MFvec3{ 0.0,-9.8,0.0 }*1.0f;
	const auto RigidBody_Gravity_Z = MFvec3{ 0.0,0.0,-9.8 }*1.0f;	

	struct RigidBodyParameters
	{
		MFtransform worldSpace{ {0},{0},{0} };
		InverseInertiaTensor iIntertiaTensorLocal{ {0},{0},{0} };
		InverseInertiaTensor iIntertiaTensorWorld{ {0},{0},{0} };
		MFquaternion orientation{ 0,0,0,1 };
		MFquaternion prevOrientation{ 0,0,0,1 };
		MFpoint3 position{ 0 };
		MFpoint3 prevPosition{ 0 };
		MFvec3 linearAcceleration{ 0 };
		MFvec3 appliedForce{ 0 };
		MFvec3 prevAcceleration{ 0 };
		MFvec3 linearVelocity{ 0 };
		MFvec3 appliedTorque{ 0 };
		MFvec3 angularVelocity{ 0 };
		MFvec3 scale{ 1 };
		MFfloat linearDamping{ 0 };
		MFfloat iMass{ 0 };
		MFfloat angularDamping{ 0 };		
	};
	void ZeroBody(RigidBodyParameters& body);

	constexpr MFu32 maxBodies{ 24 };

	class RigidBodyData : public SimulationData
	{
	public:
		MFbool Init() override;
		void Terminate() override;
		void resize(const MFsize newSize) override;		

		//used when adding bodies during runtime
		MFu32 AddBody(const RigidBodyParameters& body, const MFu64 physicsID, const MFbool dynamic);
		//used when adding bodies in bulk
		void AddBody(const MFu64 physicsID, const MFu32 bodyIndex, const MFbool dynamic);
		//returns the index to the first reserves slot
		MFu32 ReserveSpaceForBodies(const MFsize reservationSize);

		void IntegrateForces(const MFdouble& dt);
		void IntegrateVelocities(const MFdouble& dt);
		
		///GETTERS
		const MFvec3& GetLinearAccelaration(const MFu32 dataIndex) const;
		const MFvec3& GetPreviousAccelaration(const MFu32 dataIndex) const;
		const MFvec3& GetLinearVelocity(const MFu32 dataIndex) const;		
		const MFfloat& GetLinearDamping(const MFu32 dataIndex) const;
		const MFvec3& GetAngularVelocity(const MFu32 dataIndex) const;
		const MFquaternion& GetPreviousOrientation(const MFu32 dataIndex) const;
		const MFfloat& GetAngularDamping(const MFu32 dataIndex) const;
		const MFtransform& GetWorldSpace(const MFu32 dataIndex) const;
		const InverseInertiaTensor& GetLocalITensor(const MFu32 dataIndex) const;
		const InverseInertiaTensor& GetWorldITensor(const MFu32 dataIndex) const;
		const MFbool& GetDynamic(const MFu32 dataIndex) const;
		///SETTERS
		void SetLinearAccelaration(const MFvec3& linearAcceleration, const MFu32 dataIndex);
		void SetLinearVelocity(const MFvec3& linearVelocity, const MFu32 dataIndex);
		void SetLinearDamping(const MFfloat linearDamping, const MFu32 dataIndex);
		void SetAngularVelocity(const MFvec3& angularVelocity, const MFu32 dataIndex);
		void SetAngularDamping(const MFfloat angularDamping, const MFu32 dataIndex);
		void SetWorldSpace(const MFtransform& worldSpace, const MFu32 dataIndex);
		void SetLocalITensor(const InverseInertiaTensor&  localITensor, const MFu32 dataIndex);
		void SetWorldITensor(const InverseInertiaTensor&  worldITensor, const MFu32 dataIndex);
		void SetDynamic(const MFbool isDynamic, const MFu32 dataIndex);
	private:
		MFvec3* linearAcceleration;
		MFvec3* prevAcceleration;
		MFvec3* linearVelocity;		
		MFfloat* linearDamping;
		MFvec3* angularVelocity;
		MFquaternion* prevOrientation;
		MFfloat* angularDamping;
		MFtransform* worldSpace;
		InverseInertiaTensor* iIntertiaTensorLocal;
		InverseInertiaTensor* iIntertiaTensorWorld;
		MFbool* isDynamic;				
	};

	 
	
	

	inline MFtransform BodyWorldSpace(const MFmat3& worldRotation, const MFpoint3& worldPosition);
	InverseInertiaTensor InteriaWorldSpace(const MFmat3& worldRotation, const InverseInertiaTensor& iTensor);	
}
