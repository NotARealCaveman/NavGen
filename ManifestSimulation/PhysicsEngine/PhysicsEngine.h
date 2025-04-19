#pragma once
#include <vector>
#include <unordered_map>

#include <ManifestCore/Timer.h>
#include <ManifestMath/RNG.h>
#include <ManifestMath/Grid2D.h>
#include <ManifestMemory/Utility/ThreadPool.h>
#include <ManifestPersistence/DatabaseTypes.h>

#include "RigidBody.h"

using namespace Manifest_Core;//timer
using namespace Manifest_Math;
using namespace Manifest_Memory;
using Manifest_Persistence::PrimaryKey;

namespace Manifest_Simulation
{
	using RigidBodyMapIterator = decltype(std::declval<const std::unordered_map<PrimaryKey, MFu32>>().begin());

	static constexpr MFsize MAX_BODIES{ 1024 };


	class PhysicsEngine
	{
	public:
		MFbool Init(xoshiro256ss_state* _idGenerator);
		void Terminate();
		void IntegratePhysicsForces(const MFfloat dt);
		void IntegratePhysicsVelocities(const MFfloat dt); 
		void SetBodyDynamics(const MFu32& bodyIndex, const MFbool dynamic);
		MFbool IsBodyMapped(const PrimaryKey& physicsID) const;
		MFbool IsBodyDynamic(const PrimaryKey& physicsID) const;
		MFu32 GetBodyIndex(const PrimaryKey& physicsID) const;

		RigidBodyData rigidBodies;
		xoshiro256ss_state* idGenerator;
	};
}