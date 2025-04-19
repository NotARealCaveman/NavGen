#pragma once
#include <unordered_map>
#include <vector>
#include <queue>

#include <ManifestMath/Point3.h>
#include <ManifestMath/Quaternion.h>
#include <ManifestMath/Transform.h>
#include <ManifestMemory/Allocators/GlobalAllocator.h>

#include <ManifestUtility/Typenames.h>

using namespace Manifest_Math;
using namespace Manifest_Utility;
using Manifest_Memory::GlobalAllocator;

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
	 
	class SimulationData : public DataSystem
	{	
		public:
			virtual MFbool Init() = 0;
			virtual void Terminate() = 0;
			virtual void resize(const MFsize newSize) = 0;
			
			void CopySimulationDataForRendering(MFu64*& nodeID, MFquaternion*& orientation, MFpoint3*& position, MFpoint3*& previousPosition, MFvec3*& scale, const MFsize totalObjects, const MFsize offset = 0) const;
			///GETTERS
			const MFu64& GetID(const MFu32 dataIndex)const;	
			const MFquaternion& GetOrientation(const MFu32 dataIndex) const;			
			const MFpoint3& GetPosition(const MFu32 dataIndex) const;			
			const MFpoint3& GetPreviousPosition(const MFu32 dataIndex) const;
			const MFvec3& GetForce(const MFu32 dataIndex) const;			
			const MFvec3& GetTorque(const MFu32 dataIndex) const;			
			const MFvec3& GetScale(const MFu32 dataIndex) const;			
			const MFfloat& GetIMass(const MFu32 dataIndex) const;
			///SETTERS
			void SetID(const MFu64 ID, const MFu32 dataIndex);
			void SetOrientation(const MFquaternion& orientation, const MFu32 dataIndex);
			void SetPosition(const MFpoint3& position, const MFu32 dataIndex);
			void SetPreviousPosition(const MFpoint3& previousPosition, const MFu32 dataIndex);
			void SetForce(const MFvec3& force, const MFu32 dataIndex);
			void SetTorque(const MFvec3& torque, const MFu32 dataIndex);
			void SetScale(const MFvec3& scale, const MFu32 dataIndex);
			void SetIMass(const MFfloat iMass, const MFu32 dataIndex);
		protected:
			MFu64* ID;
			MFquaternion* orientation;
			MFpoint3* position;
			MFpoint3* previousPosition;
			MFvec3* appliedForce;
			MFvec3* appliedTorque;
			MFvec3* scale;
			MFfloat* iMass;
	}; 
}