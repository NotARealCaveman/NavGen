#pragma once
#include <array>
#include <unordered_set>
#include <unordered_map>
#include <deque>
#include <queue>
#include <ranges>
#include <map>
#include <set>
#include <tuple>

#include <ManifestMath/CachedVertex.h>
#include <ManifestMath/Point3.h>
#include <ManifestMath/Plane.h>
#include <ManifestMath/Triangle.h>
#include <ManifestMath/Integer3D.h>
#include <ManifestTerrain/TerrainFlags.h>

using Manifest_Math::MFpoint3, Manifest_Math::MFplane, Manifest_Math::CachedVertex, Manifest_Math::MFvec2;

namespace Manifest_Simulation
{
	static constexpr MFfloat TRAVERSABLE_TOLERANCE{ 0.95 };
	static constexpr MFfloat COPLANAR_TOLERANCE{ 0.05f };
	static constexpr MFfloat CONCAVE_TOLERANCE{ -0.1f };

	struct FreeList_NEW
	{
		static constexpr void* NO_FREELIST_ENTRY{ nullptr };

		std::unordered_map<MFsize, std::deque<void*>> openAddressesBySizeMap;

		void* allocate(const MFsize allocationSize);
		void deallocate(const MFsize allocationSize, void* address);
	};

	using NavigationIndex = MFu16;
	constexpr NavigationIndex NO_INDEX{ std::numeric_limits<NavigationIndex>::max() };

	struct NavigableEdge
	{ 
		MFpoint3 edgeOrigin{ std::numeric_limits<MFfloat>::infinity() };
		CachedVertex cachedVertex;		
		NavigationIndex edgeIndex{ NO_INDEX };		
		NavigationIndex nextIndex{ NO_INDEX };
		NavigationIndex previousIndex{ NO_INDEX };
		NavigationIndex twinIndex{ NO_INDEX };
		NavigationIndex polygonIndex{ NO_INDEX };
	}; 

	struct TraversablePortal
	{
		MFpoint3 vertexA{0};
		MFpoint3 vertexB{0};
		MFplane gate{0,0};
		NavigationIndex twinPolygonIndex{ NO_INDEX };
	};

	struct NavigablePolygon
	{
		MFplane surfacePlane{0,0};
		NavigationIndex polygonIndex{ NO_INDEX };
		NavigationIndex beginEdgeIndex{ NO_INDEX };
		std::vector<TraversablePortal> portals{};
		std::unordered_set<NavigationIndex> interiorPolygons{};

		//move these off to nav mesh gen - not needed afterwards
		std::unordered_map<CachedVertex, NavigationIndex> cachedVertexToEdgeIndexMap{};
		std::unordered_map<CachedVertex, NavigationIndex> cachedVertexToInteriorIndexMap{};
		MFsize vertexCount{0};
	};   	
}