#include "Pathing.h"

using namespace Manifest_Simulation;

MFbool PortalRanking::operator<(const PortalRanking& other) const
{	
	return distanceError < other.distanceError;
}

MFbool Manifest_Simulation::PortalDFS(const NavigationMesh& navigationMesh, const NavigablePolygon& currentPolygon, const MFpoint3& targetLocation, const MFpoint3& currentLocation, const MFvec3 currentDirection, const NavigationIndex targetPolygonIndex, std::unordered_set<NavigationIndex>& visitedPolygons, std::deque<MFpoint3>& wayPoints)
{	
	if (currentPolygon.polygonIndex == targetPolygonIndex)
		return true;
		
	if (visitedPolygons.contains(currentPolygon.polygonIndex))
		return false;
	visitedPolygons.insert(currentPolygon.polygonIndex);
	
	
	//ranks clipped portal intersection with projection along initial direction
	std::set<PortalRanking> portalRankings;	
	for (const auto& portal : currentPolygon.portals)
	{
		if (visitedPolygons.contains(portal.twinPolygonIndex))
			continue;
		 
		MFpoint3 traversableLocation;
		const MFvec3 gateSpan{ portal.vertexB - portal.vertexA };
		if (Dot(gateSpan, gateSpan) <= 0.1f)
			traversableLocation = portal.vertexA;
		else
		{
			MFpoint3 intersectionPoint;
			InteresectionLinePlane(currentLocation, currentDirection, portal.gate, intersectionPoint);
			traversableLocation = ClosestPointFromlineSegment(intersectionPoint, portal.vertexA, portal.vertexB);						
		}		
		const MFvec3 traversedPath{ traversableLocation - currentLocation };
		const MFfloat projection{ Dot(Normalize(traversedPath),currentDirection) };
		const MFfloat traversedDistance{ Dot(traversedPath,traversedPath) };
		const MFfloat distanceError{ traversedDistance - (traversedDistance * projection) };
		const MFvec3 nextDirection{ Normalize(targetLocation - traversableLocation) };

		PortalRanking portalRanking
		{ .portal {portal},.location { traversableLocation},.direction { nextDirection},.distanceError { distanceError} };
		portalRankings.insert(portalRanking);
	}
	//starting with distance delta, attempt to find a path from current polygon
	for (const PortalRanking& portalRanking : portalRankings)
	{
		const TraversablePortal portal{ portalRanking.portal };
		const NavigablePolygon& nextPolygon{ navigationMesh.polygons.at(portal.twinPolygonIndex) };
		wayPoints.push_front(portalRanking.location);
		if (PortalDFS(navigationMesh, nextPolygon, targetLocation, portalRanking.location, portalRanking.direction, targetPolygonIndex, visitedPolygons, wayPoints))
			return true;
		else
			wayPoints.pop_front();
	}		
	//origin polygon was unable to find end polygon from initial portal for search
	return false;
} 

std::vector<MFpoint3>  Manifest_Simulation::GenerateWayPoints(const BlackBoard& blackBoard, const MFpoint3& startLocation, const MFpoint3& endLocation)
{
	std::vector<MFpoint3> result;

	//determine begin and end polygons	
	const NavigationMesh& navigationMesh{ blackBoard.navigationMesh };
	NavigablePolygon const* startPolygon{ ClosestPolygonToPoint(navigationMesh,startLocation) };
	NavigablePolygon const* endPolygon{ ClosestPolygonToPoint(navigationMesh,endLocation) };

	//this needs to be fixed
	if (!endPolygon || !startPolygon)
		return{};

	//LOG({ CONSOLE_BG_CYAN },"start polygon", startPolygon->polygonIndex, "end polygon:", endPolygon->polygonIndex);

	//DFS search to determine portals required to cross
	std::unordered_set<NavigationIndex> visitedPolygons;
	std::deque<MFpoint3> wayPoints;
	const MFvec3 totalSpan{ startLocation - endLocation };	
	const auto beginTime{ std::chrono::system_clock::now() };
	if (!PortalDFS(navigationMesh,*endPolygon,startLocation,endLocation,Normalize(startLocation-endLocation),startPolygon->polygonIndex,visitedPolygons,wayPoints))
		return {};			
	const auto endTime{ std::chrono::system_clock::now() };
	//LOG({ CONSOLE_BG_CYAN }, "PATH GENERATION TIME:", endTime - beginTime);
	std::ranges::copy(wayPoints, std::back_inserter(result));	
	result.emplace_back(endLocation);
	//for (auto point : result)
	//	LOG({ CONSOLE_BG_RED }, "WAYPOINT:", point);

	TRIANGLE_SET::lock.Lock();
	TRIANGLE_SET::wayPoints = result;
	TRIANGLE_SET::wayPoints.insert(TRIANGLE_SET::wayPoints.begin(), startLocation);
	TRIANGLE_SET::lock.Unlock();
	return result;
}

