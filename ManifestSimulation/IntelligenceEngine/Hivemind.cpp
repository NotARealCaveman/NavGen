#include "Hivemind.h"

using namespace Manifest_Simulation;

 
Hivemind::Hivemind() 
{
	TableStateUpdater::UpdaterBase::blackBoard = &blackBoard;
}

Hivemind::~Hivemind()
{

}


//UPDATE TIMER IS TAKEN AS A COPY FOR NOW TO AVOID RACE CONDITION WHEN TRYING TO UPDATE IT
//IF THE UPDATE TIMER NEEDS TO BE DYNAMIC IN THE FUTURE THIS CAN BE READDRESSED
//TODO:move terrain/navmesh to external init
void Hivemind::RunAI(const MFu32 terrainReadIndex, GameTimer updateTimer)
{	
	hivemindStatus = EngineStatus::RUNNING;	
	//just sleep for a few seconds, let the databases build up
	std::this_thread::sleep_for(Manifest_Core::Timer::Duration{ 5.0s });
	while (hivemindStatus == EngineStatus::RUNNING)
	{ 
		using DesireQueue = std::priority_queue<Desire, std::vector<Desire>>;
		std::unordered_map<MFu64, DesireQueue> desireQueues;
		//REPLACE REMOVED DESIRES AND BUILD PRIORITY QUEUE WITH REMAINING 
		std::ranges::for_each(assignedDesires, [this, &desireQueues](Desire& desire)->void
		{
			static const MFu64 SENTINEL_HASH{ DesireHash(SENTINEL_DESIRE) };
			const MFu64 desireHash{ DesireHash(desire) };
			if (desireHash == SENTINEL_HASH)
				return;

			const MFbool satisfied{ ForwardFunction(desire.satisfier, blackBoard, desire.objectiveID, desire.assignedID) };
			const MFbool terminated{ ForwardFunction(desire.termiantor, blackBoard, desire.objectiveID, desire.assignedID) };
			const MFbool removed{ satisfied || terminated };
			if (removed)
			{//TECHNICALLY DOESN'T NEED TO BE ERASED BUT HELPS MEMORY REUSE
				assignedDesires.erase(DesireHash(desire));
				desire = SENTINEL_DESIRE;
			}
			else
				desireQueues[desire.assignedID].push(desire);
		});				
		//APPEND ARCHETYPE UPDATER DESIRES
		MFu32 index{ 0 };
		auto key{ blackBoard.objectAttributes.keys.begin() };
		for (Descriptor objectAttributes : blackBoard.objectAttributes)
		{
			const MFu64 objectID{ key->ID };			
			DLOG({ CONSOLE_BG_MAGENTA }, "checking constraint for object:", objectID);
			Descriptor attributeMask{ 1 };
			while (objectAttributes)
			{
				const Descriptor maskedAttributes{ objectAttributes & attributeMask };
				if (maskedAttributes == attributeMask)
				{					
					const ArchetypeUpdater& updater{ archetypeUpdaters.at(maskedAttributes) };
					const Desire desire{ updater(blackBoard,objectID,objectAttributes) };
					if (desire.representation != NO_DESIRE_SET)
						desireQueues[desire.assignedID].push(desire);
					objectAttributes ^= attributeMask;
				}
				attributeMask <<= 1;
			}
			std::advance(key, 1);			
		}

		ActionPlan* actionPlan{ new ActionPlan };
		//for each object, pick the most pressing desire to solve
		for (auto& desireQueue : desireQueues)
		{
			auto& [objectID, queue] { desireQueue };			
			MFbool solved{ false };
			while(!solved && !queue.empty())
			{
				Desire desire{ queue.top() };
				solved = SolveDesire(desire, *actionPlan);
				if (solved)
					assignedDesires.set(DesireHash(desire), std::move(desire)); 
				else//update assigned desire
					queue.pop();
			}			
		}
		//push action plan for agents to execute on simulation side		
		actionPlanTable->Push([actionPlan]()->ActionPlan*
		{return actionPlan; });

		updateTimer.Sleep();
	}	
}

MFbool Hivemind::SolveDesire(const Desire& desire, ActionPlan& actionPlan)
{
	const std::vector<ActionDetails>& actions{ droneActions.at(desire.representation) };
	const WorldState desiredState{ ForwardFunction(desire.comparativeValue,blackBoard,desire.objectiveID,desire.assignedID) };

	const FlatMap<Descriptor>& capabilities{ blackBoard.GetObjectCapabilities() };
	const Descriptor droneCapabilities{ capabilities.at(desire.assignedID) };
	for (const ActionDetails& actionDetails : actions)
	{
		//ensure end state
		if (desiredState != ForwardFunction(actionDetails.endState, blackBoard, desire.comparativeValue, desire.objectiveID, desire.assignedID))
			continue;
		//ensure capabilities
		if (std::ranges::find_if(actionDetails.capabilityCodes, [&](const Descriptor capabilityCode)->MFbool
		{
			const Descriptor capabilityMask{ droneCapabilities & capabilityCode };
			return capabilityMask == capabilityCode;
		}) == actionDetails.capabilityCodes.end())
			continue;		
		//attempt to create action - may fail requisite actions
		if (CreateAction(actionDetails, desiredState, droneCapabilities, desire.representation, desire.objectiveID, desire.assignedID, actionPlan))
			return true;
	}
	//invalid planning - clear and try again 
	actionPlan[desire.assignedID].clear();
	return false;
}

MFbool Hivemind::CreateAction(const ActionDetails& actionDetails, const WorldState desiredState, const Descriptor droneCapabilities, const Representation representation, const MFu64 objectiveID, const MFu64 assignedDroneID, ActionPlan& actionPlan)
{
	//solve all requisite states first
	for (const RequisiteState& requisiteState : actionDetails.requisiteStates)
	{
		const WorldState requiredState{ ForwardFunction(requisiteState.comparativeValue,blackBoard,objectiveID,assignedDroneID) };
		const WorldState objectState{ blackBoard.GetState(requisiteState.representation,assignedDroneID) };
		//ensure state is not currently satisfied
		if (ForwardFunction(requisiteState.satisfier, blackBoard, objectiveID, assignedDroneID))
			continue;		
		//ensure requisite state can be satisfied	
		const Desire requisiteDesire
		{
			.comparativeValue { requisiteState.comparativeValue},
			.satisfier { requisiteState.satisfier},
			.objectiveID { objectiveID},
			.assignedID { assignedDroneID },
			.representation { requisiteState.representation },
			.priority { 0 }//shouldn't matter for requisite desires
		};
		if (!SolveDesire(requisiteDesire, actionPlan))		
			return false;//not solveable - attempt new plan
	}

	//requisite's are solveable - create action
	DataSystem* goalDataSystem{ goalDataSystems.at(representation) };
	Action droneAction{ ForwardFunction(actionDetails.actionGenerator,blackBoard,desiredState,objectiveID,assignedDroneID,goalDataSystem) };
	actionPlan[assignedDroneID].emplace_back(std::move(droneAction));

	return true;
}

//technically a race condition but fuck it
void Hivemind::EndAI()
{	
	hivemindStatus = EngineStatus::CLOSING;
}


void Hivemind::AddObject(const Descriptor _objectArchetypes, const Descriptor _objectCapabilities, const MFu64 objectID)
{ 
	blackBoard.InsertObject(_objectArchetypes, _objectCapabilities, objectID);
} 

void Hivemind::RemoveObject()
{ 

}

MFbool Hivemind::FindObject()
{
	return false;
}  

MFsize Hivemind::DroneSystemCount()
{
	return hivemindDroneSystems.size();
}

const HivemindDroneSystem& Hivemind::GetDroneSystem(const Descriptor system)
{
	assert(system < hivemindDroneSystems.size());
	return *hivemindDroneSystems.at(system);
}  