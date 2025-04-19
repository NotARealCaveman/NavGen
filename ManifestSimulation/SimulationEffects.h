#pragma once
#include <array>
#include <functional>

#include <ManifestMemory/Containers/ResourceTable.h>
#include <ManifestMemory/Utility/FunctionBinder.h>
#include <ManifestSimulation/PhysicsEngine/PhysicsEngine.h>
#include <ManifestSimulation/EntityEngine/EntityEngine.h>
#include <ManifestUtility/TypeAssist.h>
#include <ManifestUtility/Typenames.h>

using namespace Manifest_Utility;
using Manifest_Memory::FunctionBinder;
using Manifest_Simulation::PhysicsEngine, Manifest_Simulation::EntityEngine;

namespace Manifest_Simulation
{
	enum class SIMULATION_EFFECTS : MFu8
	{
		APPLY_FORCE = pow2(0),		
	};

	//essentially a function binder
	using SimulationEffect = std::function<void()>;

	//lifetime unable to be made constant due to erase-remove idiom
	struct EffectApplicator
	{
		MFu64 applicatorID;
		SimulationEffect simulationEffect;		
		MFfloat elapsedTime{0.0f};//running time of effect
		MFfloat lifeTime;//total life of effect
	}; 

	struct SimulationEffectApplicators
	{			
		std::vector<EffectApplicator> physicsEffectApplicators;
		std::vector<EffectApplicator> entityEffectApplicators;
	};	
	inline void AttachEffect(std::vector<EffectApplicator>& effectApplicators, EffectApplicator&& effectApplicator);
	void UpdateEffects(const MFfloat dt, std::vector<EffectApplicator>& effectApplciators);
}