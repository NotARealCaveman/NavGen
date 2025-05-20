#pragma once
#include <ranges>
#include <vector>
#include <unordered_map>
 
#include <ManifestCore/EngineSupport.h>
#include <ManifestCore/Gametimer.h>
#include <ManifestPersistence/Runtime_Database.h>

#include "ActionPlanner.h"
#include "DronePathFinderSystem.h"

using namespace Manifest_Core;
using namespace Manifest_Persistence;


namespace Manifest_Simulation
{
	class Hivemind
	{
	public:
		Hivemind();
		~Hivemind();
		//ai loop functions
		void RunAI(const MFu32 terrainReadIndex, GameTimer updateTimer);
		void EndAI();		 
		//processing functions		
		void AddObject(const Descriptor _objectArchetypes, const Descriptor _objectCapabilities, const MFu64 objectID);
		void RemoveObject();
		MFbool FindObject(); 
		MFsize DroneSystemCount(); 
		//RTDB
		TerrainExtractionTable* terrainExtractionTable;
		ActionPlanTable* actionPlanTable;
		//runtime 
		FlatMap<std::vector<ActionDetails>> droneActions;		
		FlatMap<ArchetypeUpdater> archetypeUpdaters;
		FlatMap<DataSystem*> goalDataSystems;
		BlackBoard blackBoard;		
		FlatMap<Desire> assignedDesires;		
	private:		 
		const HivemindDroneSystem& GetDroneSystem(const Descriptor system);
		MFbool SolveDesire(const Desire& desire, ActionPlan& actionPlan);
		MFbool CreateAction(const ActionDetails& actionDetails, const WorldState desiredState, const Descriptor droneCapabilities, const Representation representation, const MFu64 objectiveID, const MFu64 assignedDroneID, ActionPlan& actionPlan);		
		std::vector<HivemindDroneSystem*> hivemindDroneSystems;		
		ActionPlan actionPlanBuffer[2];		
		MFu64 plan{ 0 };
		
		EngineStatus hivemindStatus;						
	}; 
}
