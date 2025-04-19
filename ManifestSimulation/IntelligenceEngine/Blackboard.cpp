#include "Blackboard.h"

using namespace Manifest_Simulation;
 

BlackBoard* TableStateUpdater::UpdaterBase::blackBoard{ nullptr };

void BlackBoard::Update()
{
	//update states
	for(TableStateUpdater& stateUpdater : tableStateUpdaters)
		stateUpdater.Update();	
}

void BlackBoard::InsertObject(const Descriptor _objectArchetypes, const Descriptor _objectCapabilities, const MFu64 objectID)
{
	//TODO: FREELIST	
	objectCapabilities.insert(std::make_pair(objectID, _objectCapabilities)); 
	objectAttributes.insert(std::make_pair(objectID, _objectArchetypes));
} 

const WorldState BlackBoard::GetState(const Representation representation, const  MFu64 objectID) const
{
	assert(objectStates.contains(representation));
	const FlatMap<WorldState>& states{ objectStates.at(representation) };
	if (states.contains(objectID))
		return states.at(objectID);
	else//this might be a cock in the ass in the future 
		WorldState{ 0 };
} 

//in the future this might want to be able to be batched, somehow? 
void BlackBoard::SetState(const WorldState state, const Representation representation, const  MFu64 objectID)
{	 	
	assert(objectStates.contains(representation));
	FlatMap<WorldState>& states{ objectStates.at(representation) };
	if (states.contains(objectID))
		states.at(objectID) = state;
	else
		states.insert(std::make_pair(objectID, state));
}