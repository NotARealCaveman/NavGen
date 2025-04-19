#pragma once
#include <vector>
#include <unordered_map>

#include <ManifestCore/Timer.h>

#include "EntityManager.h"
#include "EntitySpawner.h"

using Manifest_Core::Timer;
using Manifest_Math::xoshiro256ss_state;

namespace Manifest_Simulation
{	

	class EntityEngine
	{
	public:
		static constexpr MFbool ACTIVE{ true };
		static constexpr MFbool TERMINATE{ false };

		EntityEngine();
		MFbool Init(HivemindSystemEventSpace& hivemindSystemEventSpace,xoshiro256ss_state* _idGenerator);
		void Terminate();		

		///Engine Methods///		
		void UpdateEntities(const MFfloat dt, const MFu32 currentPollingGeneration);
		void IntegrateEntities(const MFfloat dt);
		EntityManager entityManager;		
	};
}