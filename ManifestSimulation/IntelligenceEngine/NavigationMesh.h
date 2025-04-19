#pragma once	
#include <array>	
#include <vector>
#include <unordered_map>
#include <numeric>
#include <ranges>

#include <ManifestMath/Plane.h>
#include <ManifestMath/Clipping.h>
#include <ManifestSimulation/EntityEngine/Entity.h>//walkable_tolerance
#include <ManifestUtility/Assert.h>
#include "NavigableStructures.h"

using Manifest_Math::CachedVertex, Manifest_Math::MFplane, Manifest_Math::MFtriangle;

namespace Manifest_Simulation
{  	 
	struct NavigationMesh
	{					
		std::unordered_set<NavigationIndex> activePolygons;

		std::vector<NavigableEdge> edges;
		std::vector<NavigablePolygon> polygons;	  		
		
		FreeList_NEW freelist;

		bool halt{ false };
	};
	void PrintState(const NavigationMesh& navigationMesh, const NavigablePolygon& navigablePolygon);
	MFbool ConfirmState(const NavigationMesh& navigationMesh, const NavigablePolygon& navigablePolygon);
	
	struct VisibleIndex
	{
		NavigationIndex edgeIndex;
		MFfloat distance;
		MFbool isReflex;

		MFbool operator<(const VisibleIndex other) const;	
	};	

	struct DelayedEdge
	{
		VisibleIndex visibleIndex;
		MFpoint3 visiblePoint;

		inline MFbool operator<(const DelayedEdge other) const
		{
			return visibleIndex < other.visibleIndex;
		}
	};

	//PORTAL GENERATION FUNCTIONS
	void GeneratePortals(NavigationMesh& navigationMesh);
	//DECOMPOSITION FUNCTIONS		
	MFbool VertexIsVisible(const MFplane incomingClippingPlane, const MFplane outgoingClippingPlane, const MFpoint3 visiblePoint);
	MFbool EdgeIsVisible(const MFvec3 incoming, const MFvec3 outgoing, const MFpoint3 origin, const MFpoint3 destination, const MFpoint3 reflexPoint, MFpoint3& visiblePoint);
	bool OnSegment(const MFvec2& p, const MFvec2& q, const MFvec2& r, const MFfloat epsilon);
	MFbool Intersection2D(const MFvec2 p1, const MFvec2 p2, const MFvec2 q1, const MFvec2 q2, MFvec2& intersectionPoint2D);
	MFbool Intersection(const MFpoint3 p1, const MFpoint3 p2, const MFpoint3 q1, const MFpoint3 q2, MFpoint3& intersectionPoint);	
	NavigablePolygon* SplitPolygon(const NavigationIndex reflexIndex, const NavigationIndex splitVertexIndex, NavigationMesh& navigationMesh);
	NavigationIndex FindSplitVertex(const NavigablePolygon& polygon, const NavigableEdge reflexEdge, NavigationMesh& navigationMesh);
	MFbool IsReflex(const std::vector<NavigableEdge>& edges, const NavigablePolygon& polygon, const NavigableEdge edge);
	NavigationIndex FindNextReflex(const NavigationMesh& navigationMesh, const NavigablePolygon& polygon);
	void QuickCompose(NavigationMesh& navigationMesh, NavigablePolygon& polygon);
	void DecomposeMesh(NavigationMesh& navigationMesh);
	//REFINEMENT FUNCTIONS
	void RefineSegment(const std::vector<NavigableEdge>& edges, const NavigationIndex startIndex, const NavigationIndex endIndex, const MFfloat epsilon, std::unordered_set<NavigationIndex>& refinedIndices);
	std::unordered_set<NavigationIndex> RamerDouglasPeucker(NavigationMesh& navigationMesh, NavigablePolygon& polygon);
	void VoxelSpaceToWorldSpace(const MFtransform& voxelToWorld, NavigationMesh& navigationMesh);
	void SimplifyMesh(NavigationMesh& navigationMesh);
	//GENERATION FUNCTIONS(EXTERIOR)
	void MergeSharedEdge(const MFu8 mergeCode, const std::array<CachedVertex, 3> cachedVertices, NavigablePolygon*& oldPolygon, NavigablePolygon*& newPolygon, NavigationMesh& navigationMesh);
	void FillMissingEdge(const std::array<CachedVertex, 3> cachedVertices, NavigablePolygon*& oldPolygon, NavigablePolygon*& newPolygon, NavigationMesh& navigationMesh);		
	void RemoveInteriorPolygon(std::array<CachedVertex, 3> cachedVertices, NavigablePolygon*& oldPolygon, NavigablePolygon*& interiorPolygon, NavigablePolygon*& newPolygon, NavigationMesh& navigationMesh);
	const MFplane ComputeNavigablePlane(const NavigablePolygon& navigablePolygon, std::vector<NavigableEdge>& edges);	
	NavigablePolygon* CreateNavigablePolygon(const MFtriangle& triangle, const std::array<CachedVertex, 3>& cachedVertices, NavigationMesh& navigationMesh);
	void InsertTriangle(const MFtriangle& triangle, NavigationMesh& navigationMesh);
	void ReserveBuffers(NavigationMesh& navigationMesh, MFu32 triangleCount); 
	//NAVIGATION MESH POSITION LOOK UP 	
	NavigablePolygon const * ClosestPolygonToPoint(const NavigationMesh& navigationMesh, const MFpoint3& point);  
}