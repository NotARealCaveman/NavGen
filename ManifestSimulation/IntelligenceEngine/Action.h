#pragma once
#include <vector>

#include <ManifestSimulation/EntityEngine/Entity.h>//EntityDataSystem

#include "Desire.h"

namespace Manifest_Simulation
{
	using Action = std::function<void(const MFu64)>;
	using ActionGenerator = std::function<Action(const BlackBoard& blackBoard, const WorldState endState, const MFu64 objectiveObjectID, const MFu64 assignedDroneID, DataSystem* goalDataSystem)>;

	struct RequisiteState
	{
		const Representation representation;
		const ComparativeValue comparativeValue;
		const Satisfier satisfier;
		const MFbool usesObjective;
	};

	using EndState = std::function<WorldState(const BlackBoard& blackBoard, const ComparativeValue comparativeValue, const MFu64 objectiveID, const MFu64 assignedID)>;
	struct ActionDetails
	{
		const ActionGenerator actionGenerator;
		const EndState endState;
		const std::vector<RequisiteState> requisiteStates;//all states need to be satisfied
		const std::vector<Descriptor> capabilityCodes{ 0 };//only one capability code needs to be satisfied
		const MFu8 cost{ 0 };
	};
}