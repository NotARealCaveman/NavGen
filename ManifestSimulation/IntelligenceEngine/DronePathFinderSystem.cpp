#include "DronePathFinderSystem.h"

using namespace Manifest_Simulation; 

DronePathFinderSystem::DronePathFinderSystem(FlatMap<std::vector<ActionDetails>>& droneActions, FlatMap<ArchetypeUpdater>& archetypeUpdaters, FlatMap<DataSystem*>& goalDataSystems, EntityPhysicsData& entityPhysicsData)
{
	AddRuntime(droneActions, archetypeUpdaters);
	goalDataSystems.insert(std::make_pair(TRANSLATION_GOAL.operableRepresenation, &entityPhysicsData));
}

void DronePathFinderSystem::AddRuntime(FlatMap<std::vector<ActionDetails>>& droneGoals, FlatMap<ArchetypeUpdater>& archetypeUpdaters)
{
	//Add system goals
	const Representation position{ TRANSLATION_GOAL.operableRepresenation };
	std::vector<ActionDetails> goals{ TRANSLATION_GOAL.goalActions };
	droneGoals.insert(std::make_pair(position, goals));
	//Add updaters	
	archetypeUpdaters.insert(std::make_pair(WANDERER, PathingUpdater));
	archetypeUpdaters.insert(std::make_pair(PATROL, PathingUpdater));
	//Register runtime
	RegisterRuntimeCapabilities(MOBILITY | FLIGHT | TELEPORTATION);
	RegisterRuntimeAttributes(WANDERER | PATROL);
}

WorldState DronePathFinderSystem::GetTargetLocation(const BlackBoard& blackBoard, const MFu64 objectiveID, const MFu64 assignedID)
{
	return blackBoard.GetState(SymbolHash("position"),objectiveID);
}


MFbool DronePathFinderSystem::AtTargetLocation(const BlackBoard& blackBoard, const MFu64 objectiveID, const MFu64 assignedID)
{
	//todo: range needs to be placed in blackboard
	const MFfloat range{ 4.0f };
	const MFpoint3 targetLocation{ blackBoard.GetState(SymbolHash("position"),objectiveID).Read<MFpoint3>() };
	const MFpoint3 currentLocation{ blackBoard.GetState(SymbolHash("position"),assignedID).Read<MFpoint3>() };
	const MFvec3 span{ targetLocation - currentLocation };
	const MFfloat distance{ Dot(span,span) };

	return distance < (range * range);
}

//CONSTRAINTS		
const MFbool DronePathFinderSystem::WanderConstraint(const BlackBoard& blackBoard, const MFu64 objectID, WorldState& constraintState)
{
	//this might not work - we'll see. previous position may need to be a system generated state. which is an interesting notion... one im not too fond of at the moment but can see myself coming around to.
	const WorldState previousPosition{ blackBoard.GetState(SymbolHash("previousPosition"),objectID) };
	const WorldState currentPosition{ blackBoard.GetState(SymbolHash("position"),objectID)};

	LOG({ CONSOLE_BG_YELLOW }, "prev:", previousPosition.Read<MFpoint3>(), "curr:", currentPosition.Read<MFpoint3>());

	const MFbool inMotion{ previousPosition != currentPosition };
	return inMotion;
}

const MFbool DronePathFinderSystem::PatrolConstraint(const BlackBoard& blackBoard, const MFu64 objectID, WorldState& constraintState)
{
	return true;
}

//UPDATERS 
const Desire DronePathFinderSystem::WanderUpdater(const BlackBoard& blackBoard, const MFu64 objectID)
{	 
	using Timer = Manifest_Core::Timer;	
	using Manifest_Core::StepTimer, Manifest_Core::TimerStopped;

	WorldState constraintState{/*compute wander point*/};
	const MFpoint3 playerPosition{ 315.0f,34.2f,-315.0f };
	constraintState = WorldState{ playerPosition };
	if (!WanderConstraint(blackBoard, objectID, constraintState))
		return Desire
	{
		.comparativeValue { [constraintState](const BlackBoard& blackBoard, const MFu64 objectiveID, const MFu64 assignedID)->WorldState { return constraintState; } },
		.satisfier { [constraintState](const BlackBoard& blackBoard, const MFu64 objectiveID, const MFu64 assignedID)->MFbool 
		{ 
			const MFpoint3 currentLocation { blackBoard.GetState(SymbolHash("position"),assignedID).Read<MFpoint3>()};
			return InRange(constraintState.Read<MFpoint3>(), currentLocation,2.0f);
		} },
		.termiantor{[timer = Timer(2.5s)](const BlackBoard& blackBoard, const MFu64 objectiveID, const MFu64 assignedID) mutable ->MFbool
		{
			StepTimer(timer);			
			return TimerStopped(timer);
		}},
		.objectiveID { objectID },
		.assignedID{ objectID },
		.representation{SymbolHash("position")},
		.priority { 0 }
	};

	return SENTINEL_DESIRE;
}

const Desire DronePathFinderSystem::PatrolUpdater(const BlackBoard& blackBoard, const MFu64 objectID)
{ 
	return SENTINEL_DESIRE;
}

//ACTIONS
Action DronePathFinderSystem::FindMobileRoute(const BlackBoard& blackBoard, const WorldState endState, const MFu64 objectiveObjectID, const MFu64 assignedDroneID, DataSystem* entityDataSystem)
{
	constexpr MFfloat AT_LOCATION_EPSILON{ 2.5 };
	Action result;	
	const WorldState dronePosition{ blackBoard.GetState(SymbolHash("position"),assignedDroneID) };
	const MFpoint3& begin{ dronePosition.Read<MFpoint3>() };
	const MFpoint3 end{ endState.Read<MFpoint3>() };
	const std::vector<MFpoint3> wayPoints{ GenerateWayPoints(blackBoard,begin,end) };
	enroute.insert(assignedDroneID);
	result = [this, wayPoints, entityDataSystem, waypointIndex = 0 ](const MFu64 droneID) mutable -> void 
	{
		EntityPhysicsData& entityData{ static_cast<EntityPhysicsData&>(*entityDataSystem) };
		const MFu32 dataIndex{ entityDataSystem->GetIDIndex(droneID) };

		if (dataIndex == DataSystem::ID_NOT_MAPPED || wayPoints.empty())
			return;

		const MFvec3 XZclip{ 1,0,1 };
		MFpoint3 dronePosition{ ComponentMultiply(entityData.GetPosition(dataIndex),XZclip) };
		MFpoint3 targetLocation{ ComponentMultiply(wayPoints.at(waypointIndex),XZclip) };

		const MFbool inRange{ InRange(targetLocation,dronePosition,1.0f) };
		if (inRange) {
			waypointIndex = std::min(static_cast<MFsize>(waypointIndex + 1), wayPoints.size() - 1);
			targetLocation = wayPoints.at(waypointIndex);
		}
		//if in range of final point, negate bit to 0 and set no moving direction
		const MFbool modifier{ !(inRange && waypointIndex == wayPoints.size() - 1) };
		//clip to xy plane
		const MFvec3 movingDirection{ targetLocation - dronePosition};
		entityData.SetMovingDirection(Normalize(targetLocation - dronePosition)* modifier, dataIndex);
	};
	return result;
}

const MFbool Manifest_Simulation::InRange(const MFpoint3& targetLocation, const MFpoint3 dronePositoin, const MFfloat permissibleDistance)
{
	const MFvec3 centerSpan{ targetLocation - dronePositoin };
	//check if agent is close enough to location
	return Dot(centerSpan, centerSpan) <= (permissibleDistance * permissibleDistance);
}