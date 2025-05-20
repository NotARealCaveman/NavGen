#pragma once

#include "Blackboard.h"

namespace Manifest_Simulation
{
	using ComparativeValue = std::function<WorldState(const BlackBoard& blackBoard, const MFu64 objectiveID, const MFu64 assignedID)>;
	using Satisfier = std::function<MFbool(const BlackBoard& blackBoard, const MFu64 objectiveID, const MFu64 assignedID)>;
	using Terminator = Satisfier;

	const inline Terminator MUST_COMPLETE = [](const BlackBoard& blackBoard, const MFu64 objectiveID, const MFu64 assignedID)->MFbool { return false; };
	const inline Terminator TERMINATE = [](const BlackBoard& blackBoard, const MFu64 objectiveID, const MFu64 assignedID)->MFbool { return true; };

	struct Desire
	{
		ComparativeValue comparativeValue;
		Satisfier satisfier;
		Terminator termiantor{ MUST_COMPLETE };
		MFu64 objectiveID;
		MFu64 assignedID;
		Representation representation;
		MFu8 priority;

		inline const MFbool operator<(const Desire& other) const { return priority < other.priority; }
	};
	extern Desire SENTINEL_DESIRE;
	constexpr Representation NO_DESIRE_SET{ 0 };
	//evalutes object contsraints based on archetypes - should an updater dected a violated constraint a desire to restore the object is returned
	using ArchetypeUpdater = std::function<Desire(const BlackBoard& blackBoard, const MFu64 objectID, const MFu64 objectAttributes)>;

	struct DesireHash
	{		 
	public:
		DesireHash(const Desire& desire) : hash{ hash3_u64(desire.representation, desire.objectiveID, desire.assignedID) } {}

		operator MFu64() const { return hash; }

	private:
		static constexpr MFu64 GOLDEN_RATIO = 0x9e3779b97f4a7c15;   

		static uint64_t hash3_u64(uint64_t representation, uint64_t objectiveID, uint64_t assignedID)
		{
			representation ^= objectiveID + GOLDEN_RATIO + (representation << 12) + (representation >> 4);
			representation ^= assignedID + GOLDEN_RATIO + (representation << 12) + (representation >> 4);

			return representation;
		}

		const MFu64 hash;
	};	
}