#include "DataSystem.h"

using namespace Manifest_Simulation;
 
DataSystem::DataSystem()
	:IDtoIndexMap{&GlobalAllocator::ThreadMemoryPool()}, freeList{ &GlobalAllocator::ThreadMemoryPool() }
{

}

MFu32 DataSystem::GetIDIndex(const MFu64 ID) const
{
	return IDtoIndexMap.contains(ID) ?  IDtoIndexMap.at(ID) : ID_NOT_MAPPED;
}

const std::pmr::unordered_map<MFu64, MFu32>& DataSystem::GetIDIndexMap()
{
	return IDtoIndexMap;
}

const MFsize DataSystem::size() const
{
	return activeSize;
}

void DataSystem::erase(const std::vector<MFu64>& removedIDs)
{
	std::ranges::for_each(removedIDs, [this](const MFu64 removedID)
	{
		IDtoIndexMap.erase(removedID);
	});
	std::ranges::copy(removedIDs, std::back_inserter(freeList));
}

void SimulationData::CopySimulationDataForRendering(MFu64*& nodeID, MFquaternion*& orientation, MFpoint3*& position, MFpoint3*& previousPosition, MFvec3*& scale, const MFsize totalObjects, const MFsize offset) const
{
	memcpy(&orientation[offset], this->orientation, sizeof(MFquaternion) * totalObjects);
	memcpy(&position[offset], this->position, sizeof(MFpoint3) * totalObjects);
	memcpy(&previousPosition[offset], this->previousPosition, sizeof(MFpoint3) * totalObjects);
	memcpy(&nodeID[offset], this->ID, sizeof(MFu64) * totalObjects);
	memcpy(&scale[offset], this->scale, sizeof(MFvec3) * totalObjects);
}


const MFu64& SimulationData::GetID(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return ID[dataIndex];
}
void SimulationData::SetID(const MFu64 ID, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->ID[dataIndex] = ID;
}
const MFquaternion& SimulationData::GetOrientation(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return orientation[dataIndex];
}
void SimulationData::SetOrientation(const MFquaternion& orientation, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->orientation[dataIndex] = orientation;
 }
const MFpoint3& SimulationData::GetPosition(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return position[dataIndex];
}
void SimulationData::SetPosition(const MFpoint3& position, const MFu32 dataIndex)
{
	assert(dataIndex < size());	

	this->position[dataIndex] = position;
}
const MFpoint3& SimulationData::GetPreviousPosition(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return previousPosition[dataIndex];
}
void SimulationData::SetPreviousPosition(const MFpoint3& previousPosition, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->previousPosition[dataIndex] = previousPosition;
}
const MFvec3& SimulationData::GetForce(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return appliedForce[dataIndex];
}
void SimulationData::SetForce(const MFvec3& force, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	appliedForce[dataIndex] = force;
}
const MFvec3& SimulationData::GetTorque(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return appliedTorque[dataIndex];
}
void SimulationData::SetTorque(const MFvec3& torque, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	appliedTorque[dataIndex] = torque;
}
const MFvec3& SimulationData::GetScale(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return scale[dataIndex];
}
void SimulationData::SetScale(const MFvec3& scale, const MFu32 dataIndex)
{
	assert(dataIndex < size());

	this->scale[dataIndex] = scale;
}
const MFfloat& SimulationData::GetIMass(const MFu32 dataIndex) const
{
	assert(dataIndex < size());

	return iMass[dataIndex];
}
void SimulationData::SetIMass(const MFfloat iMass, const MFu32 dataIndex)
{
	assert(dataIndex < size());
}