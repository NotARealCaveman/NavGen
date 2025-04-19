#pragma once
#include <set>

#include "NavigationMesh.h"

using namespace Manifest_Math;

namespace Manifest_Simulation
{
	struct PortalRanking
	{
		//pairs polygon index with portal index to access polygon
		TraversablePortal portal;
		MFpoint3 location;
		MFvec3 direction;
		MFfloat distanceError;
		MFbool operator<(const PortalRanking& other) const;
	};
	//returns true if DFS was able to find the end polygon
	MFbool PortalDFS(const NavigationMesh& navigationMesh, const NavigablePolygon& currentPolygon, const MFpoint3& targetLocation, const MFpoint3& currentLocation, const MFvec3 currentDirection, const NavigationIndex targetPolygonIndex, std::unordered_set<NavigationIndex>& visitedPolygons, std::deque<MFpoint3>& wayPoints);
	std::vector<MFpoint3> GenerateWayPoints(const NavigationMesh& navigationMesh, const MFpoint3& startLocation, const MFpoint3& endLocation);
}