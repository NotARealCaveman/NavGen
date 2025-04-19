#pragma once
#include <functional>
#include <deque>

#include "ActionPlanner.h"
#include "Goal.h"

namespace Manifest_Simulation
{		
	class HivemindDroneSystem
	{
	public:
		HivemindDroneSystem() = default;			
	protected:		 	
		virtual void AddRuntime(FlatMap<std::vector<ActionDetails>>& droneGoals, FlatMap<ArchetypeUpdater>& archetypeUpdaters) = 0;		
		void RegisterRuntimeCapabilities(const Descriptor runtimeCapabilities);
		void RegisterRuntimeAttributes(const Descriptor runtimeArchetypes);
	private:
		static Descriptor RUNTIME_CAPABILITIES;
		static Descriptor RUNTIME_ATTRIBUTES;
	};	
}