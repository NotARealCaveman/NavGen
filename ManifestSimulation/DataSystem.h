#pragma once
#include <unordered_map>
#include <vector>
#include <queue>

#include <ManifestMath/Point3.h> 
#include <ManifestMath/Transform.h> 

#include <ManifestUtility/Typenames.h>

using namespace Manifest_Math;
using namespace Manifest_Utility; 

namespace Manifest_Simulation
{
	class DataSystem
	{
	public:
		static constexpr MFu32 ID_NOT_MAPPED{ std::numeric_limits<MFu32>::max() };

		DataSystem();
		virtual MFbool Init() = 0;
		virtual void Terminate() = 0;
		virtual void resize(const MFsize newSize) = 0;
		const MFsize size() const;
		void erase(const std::vector<MFu64>& removedIDs);
		//returns Index to mapped ID if it exists		
		MFu32 GetIDIndex(const MFu64 ID) const;
		const std::pmr::unordered_map<MFu64, MFu32>& GetIDIndexMap();

	protected:
		std::pmr::unordered_map<MFu64, MFu32> IDtoIndexMap;
		std::pmr::deque<MFu32> freeList;
		MFsize activeSize{ 0 };
		MFsize capacity{ 0 };
	}; 
}