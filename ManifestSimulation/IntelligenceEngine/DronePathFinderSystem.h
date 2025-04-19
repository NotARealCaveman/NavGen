#pragma once
#include <unordered_map>

#include <ManifestCore/Timer.h>
#include <ManifestPersistence/Runtime_Database.h>

#include "Pathing.h"
#include "HivemindDroneSystem.h"

namespace Manifest_Simulation
{ 	
	 
	//initial hivemind agent system supplied by the framework.
	//most if not all other derived agent systems will require some form of translation, thus pathfinding is made to be a core part of the framework
	class DronePathFinderSystem : public HivemindDroneSystem
	{
	public:		
		DronePathFinderSystem() = default;
		DronePathFinderSystem(FlatMap<std::vector<ActionDetails>>& droneActions, FlatMap<ArchetypeUpdater>& archetypeUpdaters, FlatMap<DataSystem*>& goalDataSystems, EntityPhysicsData& entityPhysicsData);

		static WorldState GetTargetLocation(const BlackBoard& blackBoard, const MFu64 objectiveID, const MFu64 assignedID);		
		static MFbool AtTargetLocation(const BlackBoard& blackBoard, const MFu64 objectiveID, const MFu64 assignedID);
	private:
		static constexpr MFu8 MOBILE{ 0 };
		static constexpr MFu8 FLYING{ MOBILE + 1 };
		static constexpr MFu8 TELEPORTING{ FLYING + 1 };		
		static constexpr MFu8 WANDERING{ 0 };
		static constexpr MFu8 PATROLING{ WANDERING + 1 };
	public:
		//CAPABILITIES
		constexpr static Descriptor MOBILITY{ pow2(MOBILE) };
		constexpr static Descriptor FLIGHT{ pow2(FLYING) };
		constexpr static Descriptor TELEPORTATION{ pow2(TELEPORTING) };
		//ATTRIBUTES		
		constexpr static Descriptor WANDERER{ pow2(WANDERING) };
		constexpr static Descriptor PATROL{ pow2(PATROLING) };				
	private:
		void AddRuntime(FlatMap<std::vector<ActionDetails>>& droneActions, FlatMap<ArchetypeUpdater>& archetypeUpdaters) override;
		DronePathFinderSystem(const DronePathFinderSystem&) = delete;	

		Action FindMobileRoute(const BlackBoard& blackBoard, const WorldState endState, const MFu64 objectiveObjectID, const MFu64 assignedDroneID, DataSystem* entityDataSystem);

		const std::vector<ActionDetails> TRANSLATION_ACTIONS
		{
			ActionDetails{//mobile translation
				.actionGenerator { [this](const BlackBoard& blackBoard, const WorldState endState, const MFu64 objectiveObjectID, const MFu64 assignedDroneID, DataSystem* entityDataSystem)->Action {return FindMobileRoute(blackBoard,endState,objectiveObjectID,assignedDroneID,entityDataSystem); }},
				.endState {[this](const BlackBoard& blackBoard, const ComparativeValue comparativeValue, const MFu64 objectiveID, const MFu64 assignedID)->WorldState {return GetTargetLocation(blackBoard,objectiveID,assignedID); }},
				.capabilityCodes { MOBILITY }, .cost { 1 }},
			ActionDetails{//wander translation
				.actionGenerator { [this](const BlackBoard& blackBoard, const WorldState endState, const MFu64 objectiveObjectID, const MFu64 assignedDroneID, DataSystem* entityDataSystem)->Action {return FindMobileRoute(blackBoard,endState,objectiveObjectID,assignedDroneID,entityDataSystem); }},
				.endState {[this](const BlackBoard& blackBoard, const ComparativeValue comparativeValue, const MFu64 objectiveID, const MFu64 assignedID)->WorldState {return ForwardFunction(comparativeValue,blackBoard,objectiveID,assignedID); }},
				.capabilityCodes { MOBILITY }, .cost { 2 }},
		}; 		

		//Goal will solve position desires ending at the desire value
		const Goal TRANSLATION_GOAL
		{
			.operableRepresenation{SymbolHash("position")},
			.goalActions{ TRANSLATION_ACTIONS },			
		};
				
		//CONSTRAINTS		
		static const MFbool WanderConstraint(const BlackBoard& blackBoard, const MFu64 objectID, WorldState& constraintState);
		static const MFbool PatrolConstraint(const BlackBoard& blackBoard, const MFu64 objectID, WorldState& constraintState);
		//UPDATERS		
		static const Desire WanderUpdater(const BlackBoard& blackBoard, const MFu64 objectID);
		static const Desire PatrolUpdater(const BlackBoard& blackBoard, const MFu64 objectID);
		std::unordered_set<MFu64> enroute;
		const ArchetypeUpdater PathingUpdater = [this](const BlackBoard& blackBoard, const MFu64 objectID, const MFu64 objectAttributes)->Desire
		{ 
			if ((objectAttributes & WANDERER) == WANDERER)
				return WanderUpdater(blackBoard, objectID);
			else if((objectAttributes & PATROL) == PATROL)
				return PatrolUpdater(blackBoard, objectID);
		};
	};

	const MFbool InRange(const MFpoint3& targetLocation, const MFpoint3 dronePosition, const MFfloat permissibleDistance);
}