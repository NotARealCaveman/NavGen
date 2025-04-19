#include "NavigationMesh.h"

using namespace Manifest_Simulation;
using namespace Manifest_Math;
 
MFbool VisibleIndex::operator<(const VisibleIndex other) const
{
	if (isReflex && !other.isReflex)
		return true;
	if (other.isReflex && !isReflex)
		return false;

	return distance < other.distance;
};

void Manifest_Simulation::GeneratePortals(NavigationMesh& navigationMesh)
{
	//number of units to push in the portal vertices so pathing doesn't send an agent off the edge
	const MFfloat PORTAL_VERTEX_EPSILON{ 3.0f };

	for (const NavigationIndex polygonIndex : navigationMesh.activePolygons)
	{
		NavigablePolygon& polygon{ navigationMesh.polygons.at(polygonIndex) };
		NavigationIndex edgeIndex{ polygon.beginEdgeIndex };
		do
		{
			const NavigableEdge& edge{ navigationMesh.edges.at(edgeIndex) };
			if (edge.twinIndex != NO_INDEX)
			{				
				const NavigableEdge& twin{ navigationMesh.edges.at(edge.twinIndex) };
				const MFvec3 incoming{ Normalize(twin.edgeOrigin - edge.edgeOrigin) };
				const MFvec3 edgeNormal{ Cross(polygon.surfacePlane.Normal(),incoming) };
				const MFfloat offset{ -Dot(edge.edgeOrigin,edgeNormal) };
				const MFplane plane{ Normalize(MFplane{edgeNormal,offset}) };		
				const MFpoint3 vertexA{ edge.edgeOrigin + incoming * PORTAL_VERTEX_EPSILON };
				const MFpoint3 vertexB{ twin.edgeOrigin - incoming * PORTAL_VERTEX_EPSILON };
				polygon.portals.emplace_back(TraversablePortal{ .vertexA { vertexA}, .vertexB{vertexB}, .gate{plane}, .twinPolygonIndex{twin.polygonIndex} });
			}
			edgeIndex = edge.nextIndex;
		} while (edgeIndex != polygon.beginEdgeIndex);
	}
}

NavigablePolygon* Manifest_Simulation::SplitPolygon(const NavigationIndex reflexIndex, const NavigationIndex splitVertexIndex, NavigationMesh& navigationMesh)
{	
	//create new polygon
	void* polygonAdress{ navigationMesh.freelist.allocate(sizeof(NavigablePolygon)) };
	NavigablePolygon* splitPolygon{ nullptr };
	if (polygonAdress == FreeList_NEW::NO_FREELIST_ENTRY)
		splitPolygon = &navigationMesh.polygons.emplace_back();
	else
		splitPolygon = new(polygonAdress)NavigablePolygon;
	//calculate new polygon index
	splitPolygon->polygonIndex = std::distance(navigationMesh.polygons.data(), splitPolygon);
	navigationMesh.activePolygons.insert(splitPolygon->polygonIndex);	
	//create R' 
	void* rPrimeAddress{ navigationMesh.freelist.allocate(sizeof(NavigableEdge)) };
	NavigableEdge* rPrime{ nullptr };
	if (rPrimeAddress == FreeList_NEW::NO_FREELIST_ENTRY)
		rPrime = &navigationMesh.edges.emplace_back();
	else
		rPrime = new(rPrimeAddress)NavigableEdge;
	//create S'	
	void* sPrimeAddress{ navigationMesh.freelist.allocate(sizeof(NavigableEdge)) };
	NavigableEdge* sPrime{ nullptr };
	if (sPrimeAddress == FreeList_NEW::NO_FREELIST_ENTRY)
		sPrime = &navigationMesh.edges.emplace_back();
	else
		sPrime = new(sPrimeAddress)NavigableEdge;

	//update new edges
	NavigableEdge& reflexEdge{ navigationMesh.edges.at(reflexIndex) };
	NavigableEdge& splitEdge{ navigationMesh.edges.at(splitVertexIndex) };
	NavigablePolygon& originalPolygon{ navigationMesh.polygons.at(reflexEdge.polygonIndex) };
	*rPrime = reflexEdge;
	*sPrime = splitEdge;	
	rPrime->edgeIndex = std::distance(navigationMesh.edges.data(), rPrime);
	sPrime->edgeIndex = std::distance(navigationMesh.edges.data(), sPrime);
	rPrime->nextIndex = sPrime->edgeIndex;
	sPrime->previousIndex = rPrime->edgeIndex;
	NavigableEdge& sPrimeNext{ navigationMesh.edges.at(sPrime->nextIndex) };
	sPrimeNext.previousIndex = sPrime->edgeIndex;
	NavigableEdge& rPrimePrevious{ navigationMesh.edges.at(rPrime->previousIndex) };
	rPrimePrevious.nextIndex = rPrime->edgeIndex;
	
	reflexEdge.previousIndex = splitEdge.edgeIndex;
	splitEdge.nextIndex = reflexEdge.edgeIndex;
	rPrime->twinIndex = splitEdge.edgeIndex;	
	ASSERT(reflexEdge.twinIndex == NO_INDEX);
	if (splitEdge.twinIndex != NO_INDEX)
	{
		NavigableEdge& oldTwin{ navigationMesh.edges.at(splitEdge.twinIndex) };		
		oldTwin.twinIndex = sPrime->edgeIndex;
		sPrime->twinIndex = oldTwin.edgeIndex;
	}
	splitEdge.twinIndex = rPrime->edgeIndex;	

	//update pointers
	splitPolygon->beginEdgeIndex = rPrime->edgeIndex;
	splitPolygon->surfacePlane = originalPolygon.surfacePlane;
	originalPolygon.beginEdgeIndex = reflexIndex;
	NavigationIndex edgeIndex{ splitPolygon->beginEdgeIndex };
	do
	{
		NavigableEdge& edge{ navigationMesh.edges.at(edgeIndex) };
		edge.polygonIndex = splitPolygon->polygonIndex;
		edgeIndex = edge.nextIndex;
	} while (edgeIndex != splitPolygon->beginEdgeIndex); 
	 

	return splitPolygon;
}
  
MFbool Manifest_Simulation::VertexIsVisible(const MFplane incomingClippingPlane, const MFplane outgoingClippingPlane, const MFpoint3 visiblePoint)
{ 
	const MFfloat distanceToIncomingPlane{ Dot(incomingClippingPlane,visiblePoint) };
	const MFbool isVisibleFromIncoming{ distanceToIncomingPlane >= -1.0e-6f };
	const MFfloat distanceToOutgoingPlane{ Dot(outgoingClippingPlane,visiblePoint) };
	const MFbool isVisibleFromOutgoing{ distanceToOutgoingPlane >= -1.0e-6f };

	return static_cast<MFbool>(isVisibleFromIncoming & isVisibleFromOutgoing);
}

MFbool Manifest_Simulation::EdgeIsVisible(const MFvec3 incoming, const MFvec3 outgoing,const MFpoint3 origin, const MFpoint3 destination, const MFpoint3 reflexPoint, MFpoint3& visiblePoint)
{
	int p = 100;
	for (auto i = 0; i < 3; ++i)
	{
		p *= 10;
		MFpoint3 incomingIntersectionPoint;
		const MFpoint3 incomingPoint{ reflexPoint + Normalize(incoming) * p };
		const MFbool intersectsIncoming{ Intersection(origin,destination,reflexPoint,incomingPoint, incomingIntersectionPoint) };
		MFpoint3 outgoingIntersectionPoint;		
		const MFpoint3 outgoingPoint{ reflexPoint - Normalize(outgoing) * p };
		const MFbool intersectsOutgoing{ Intersection(origin,destination,outgoingPoint, reflexPoint,outgoingIntersectionPoint) };
		if (intersectsIncoming && intersectsOutgoing)
		{
			visiblePoint = (incomingIntersectionPoint + outgoingIntersectionPoint) * 0.5f;

			return true;
		}
	}

	return false;
}

// Helper function to check if a point is on a segment in 2D
bool Manifest_Simulation::OnSegment(const MFvec2& p, const MFvec2& q, const MFvec2& r, const MFfloat epsilon)
{	
	const MFfloat Mx{ std::fmaxf(p.x, r.x) + epsilon };
	const MFfloat mx{ std::fminf(p.x, r.x) - epsilon };
	const MFfloat My{ std::fmaxf(p.y, r.y) + epsilon };
	const MFfloat my{ std::fminf(p.y, r.y) - epsilon };
	return q.x <= Mx && q.x >= mx &&
		q.y <= My && q.y >= my;
}

// 2D intersection function in XZ plane
MFbool Manifest_Simulation::Intersection2D(const MFvec2 p1, const MFvec2 p2, const MFvec2 q1, const MFvec2 q2, MFvec2& intersectionPoint2D)
{
	const MFfloat epsilon{ 1e-4 };	
	const MFfloat a1 = p2.y - p1.y;
	const MFfloat b1 = p1.x - p2.x;
	const MFfloat c1 = a1 * p1.x + b1 * p1.y;
	const MFfloat a2 = q2.y - q1.y;
	const MFfloat b2 = q1.x - q2.x;
	const MFfloat c2 = a2 * q1.x + b2 * q1.y;
	const MFfloat det = a1 * b2 - a2 * b1;

	// Check if lines are not parallel
	if (std::fabs(det) > epsilon)
	{
		intersectionPoint2D.x = (b2 * c1 - b1 * c2) / det;
		intersectionPoint2D.y = (a1 * c2 - a2 * c1) / det;

		// Check if intersection is on both segments
		if (OnSegment(p1, intersectionPoint2D, p2, epsilon) && OnSegment(q1, intersectionPoint2D, q2, epsilon))
			return true;		
	}

	// Return sentinel value if no intersection
	return false;
}

// Main 3D intersection function for y = 1 plane
MFbool Manifest_Simulation::Intersection(const MFpoint3 p1, const MFpoint3 p2, const MFpoint3 q1, const MFpoint3 q2, MFpoint3& intersectionPoint)
{
	// Project points to the XZ plane
	const MFvec2 pp1 { p1.x, p1.z };
	const MFvec2 pp2 { p2.x, p2.z };
	const MFvec2 qq1 { q1.x, q1.z };
	const MFvec2 qq2 { q2.x, q2.z };

	// Calculate 2D intersection
	MFvec2 interesctionPoint2D;  
	if (!Intersection2D(pp1, pp2, qq1, qq2, interesctionPoint2D))
		return false;

	// Return the intersection point
	intersectionPoint = MFpoint3{ interesctionPoint2D.x, p1.y, interesctionPoint2D.y };

	return true;
}

NavigationIndex Manifest_Simulation::FindSplitVertex(const NavigablePolygon& polygon, const NavigableEdge reflexEdge, NavigationMesh& navigationMesh)
{
	std::vector<NavigableEdge>& edges{ navigationMesh.edges };  	
	std::set<VisibleIndex> visibleIndices;
	std::set<DelayedEdge> delayedEdges;
	//compute visibility clipping planes		
	const MFpoint3 reflexPoint{ reflexEdge.edgeOrigin };
	const MFvec3 incoming{ reflexPoint - edges.at(reflexEdge.previousIndex).edgeOrigin }; 
	const MFvec3 outgoing{ edges.at(reflexEdge.nextIndex).edgeOrigin - reflexPoint };	
	const MFvec3 surfaceNormal{ Cross(outgoing,incoming)};
	const MFvec3 incomingNormal{ Cross(surfaceNormal,incoming) };
	const MFvec3 outgoingNormal{ Cross(surfaceNormal,outgoing) };
	const MFplane incomingClippingPlane{ Normalize(MFplane{incomingNormal,-Dot(incomingNormal,reflexPoint)}) };
	const MFplane outgoingClippingPlane{ Normalize(MFplane{outgoingNormal,-Dot(outgoingNormal,reflexPoint)}) };
	 
	MFpoint3 intersectionPoint;
	NavigationIndex edgeIndex{ reflexEdge.nextIndex };	
	do
	{ 
		const NavigableEdge current{ edges.at(edgeIndex) };
		const MFpoint3 origin{ current.edgeOrigin };
		const NavigableEdge next{ edges.at(current.nextIndex) };
		const MFpoint3 destination{ next.edgeOrigin };
		//avoid pruning cache if next edge is reflex edge
		if (next.edgeIndex != reflexEdge.edgeIndex)
		{
			//remove any visible indices that intersect the current edge	
			for (auto indexIterator{ visibleIndices.begin() }; indexIterator != visibleIndices.end();)
			{
				const MFpoint3 visiblePoint{ edges.at(indexIterator->edgeIndex).edgeOrigin };
				if (Intersection(origin, destination, reflexPoint, visiblePoint, intersectionPoint))
					indexIterator = visibleIndices.erase(indexIterator);
				else
					++indexIterator;
			}
			//remove any delayed edges that intersect the current edge		
			for (auto edgeIterator{ delayedEdges.begin() }; edgeIterator != delayedEdges.end();)
			{
				const DelayedEdge delayedEdge{ *edgeIterator };
				const MFpoint3 visiblePoint{ delayedEdge.visiblePoint };
				if (Intersection(origin, destination, reflexPoint, visiblePoint, intersectionPoint))
					edgeIterator = delayedEdges.erase(edgeIterator);
				else
					++edgeIterator;
			}
		}
		//if the current point is visible, add to visible indices
		if (VertexIsVisible(incomingClippingPlane, outgoingClippingPlane, origin))
		{			
			const VisibleIndex visibleIndex
			{
				.edgeIndex { edgeIndex},
				.distance { Dot(reflexPoint - origin, reflexPoint - origin)},
				.isReflex { IsReflex(edges, polygon, current)}
			};
			visibleIndices.insert(visibleIndex);
		}//if point is not visible but edge is, delay for future processing	
		else if (EdgeIsVisible(incoming,outgoing,origin,destination,reflexPoint,intersectionPoint))
		{
			DelayedEdge delayedEdge
			{
				.visibleIndex
				{
					.edgeIndex = edgeIndex,
					.distance = Dot(reflexPoint - origin, reflexPoint - origin),
					.isReflex = IsReflex(edges, polygon, current)
				},
				.visiblePoint { intersectionPoint}
			};
			delayedEdges.insert(delayedEdge);
		}
		edgeIndex = next.edgeIndex;
	} while (edgeIndex != reflexEdge.edgeIndex);
	
	NavigationIndex splitIndex{ NO_INDEX };
	//if no visible point was found use a delayed edge
	if (!visibleIndices.empty())
		splitIndex = visibleIndices.begin()->edgeIndex;	
	else
	{
		ASSERT(delayedEdges.size());
		auto delayedEdge{ delayedEdges.begin() };
		NavigableEdge& origin{ edges.at(delayedEdge->visibleIndex.edgeIndex) };
		NavigableEdge& destination{ edges.at(origin.nextIndex) };
		void* edgeAddress{ navigationMesh.freelist.allocate(sizeof(NavigableEdge)) };
		NavigableEdge* newEdge{ nullptr };
		if (edgeAddress == FreeList_NEW::NO_FREELIST_ENTRY)
			newEdge = &navigationMesh.edges.emplace_back();
		else
			newEdge = new(edgeAddress)NavigableEdge;
		newEdge->edgeOrigin = delayedEdge->visiblePoint;		
		newEdge->polygonIndex = reflexEdge.polygonIndex;
		newEdge->edgeIndex = std::distance(navigationMesh.edges.data(), newEdge);
		newEdge->previousIndex = origin.edgeIndex;
		newEdge->nextIndex = destination.edgeIndex;
		destination.previousIndex = newEdge->edgeIndex;
		origin.nextIndex = newEdge->edgeIndex;
		splitIndex = newEdge->edgeIndex;		
	}	
	ASSERT(splitIndex != NO_INDEX);

	return splitIndex;
}

MFbool Manifest_Simulation::IsReflex(const std::vector<NavigableEdge>& edges, const NavigablePolygon& polygon, const NavigableEdge edge)
{
	const MFvec3 normal{ polygon.surfacePlane.Normal() };
	const MFvec3 incoming{ edge.edgeOrigin - edges.at(edge.previousIndex).edgeOrigin };
	const MFvec3 outgoing{ edges.at(edge.nextIndex).edgeOrigin - edge.edgeOrigin };
	const MFvec3 incomingXoutgoing{ Normalize(Cross(incoming,outgoing)) };
	const MFfloat projection{ Dot(normal,incomingXoutgoing) };
	const MFbool isReflex{ projection < 0.0f };

	return isReflex;
}

NavigationIndex Manifest_Simulation::FindNextReflex(const NavigationMesh& navigationMesh, const NavigablePolygon& polygon)
{
	NavigationIndex edgeIndex{ polygon.beginEdgeIndex };
	do
	{
		const NavigableEdge& current{ navigationMesh.edges.at(edgeIndex) };
		if (IsReflex(navigationMesh.edges, polygon,current))
			return edgeIndex;		

		edgeIndex = current.nextIndex;
	} while (edgeIndex != polygon.beginEdgeIndex);

	return NO_INDEX;
}

void Manifest_Simulation::QuickCompose(NavigationMesh& navigationMesh, NavigablePolygon& polygon)
{	
	PrintState(navigationMesh, polygon);
	NavigationIndex reflexIndex{ FindNextReflex(navigationMesh,polygon) };	
	while (reflexIndex != NO_INDEX)
	{
		const NavigableEdge& reflexEdge{ navigationMesh.edges.at(reflexIndex) };
		const NavigationIndex splitVertexIndex{ FindSplitVertex(polygon, reflexEdge,navigationMesh) }; 	
		ASSERT(splitVertexIndex != NO_INDEX); 	
		NavigablePolygon& splitPolygon{ *SplitPolygon(reflexIndex,splitVertexIndex,navigationMesh) };
		QuickCompose(navigationMesh, splitPolygon);
		reflexIndex = FindNextReflex(navigationMesh, polygon);
	}
	polygon.surfacePlane = ComputeNavigablePlane(polygon, navigationMesh.edges);
}

  
void Manifest_Simulation::DecomposeMesh(NavigationMesh& navigationMesh)
{ 
	std::vector<NavigableEdge>& edges{ navigationMesh.edges };	
	auto temp{ navigationMesh.activePolygons };	
	for (const NavigationIndex polygonIndex : temp)
	{				
		NavigablePolygon& polygon{ navigationMesh.polygons.at(polygonIndex) };				
		if (polygon.vertexCount <= 3)
			continue;
		system("cls");
		QuickCompose(navigationMesh, polygon);
	} 
} 

void Manifest_Simulation::RefineSegment(const std::vector<NavigableEdge>& edges, const NavigationIndex startIndex, const NavigationIndex endIndex, const MFfloat epsilon, std::unordered_set<NavigationIndex>& refinedIndices)
{		
	const NavigableEdge& startEdge{ edges.at(startIndex) };
	const NavigableEdge& endEdge{ edges.at(endIndex) };

	//find point furthest from current line 
	NavigationIndex bestIndex{ endIndex };
	NavigationIndex bestDistance{ 0 };
	NavigationIndex edgeIndex{ startIndex };
	do
	{
		const NavigableEdge& edge{ edges.at(edgeIndex) };
		const MFpoint3 closestPoint{ ClosestPointFromlineSegment(edge.edgeOrigin,startEdge.edgeOrigin,endEdge.edgeOrigin) };
		const MFvec3 span{ closestPoint - edge.edgeOrigin };
		const MFfloat distance{ Dot(span,span) };
		if (distance > bestDistance)
		{
			bestDistance = distance;
			bestIndex = edgeIndex;
		}
		edgeIndex = edge.nextIndex;
	} while (edgeIndex != endIndex);

	//check if furthest point is significant and requires spliting
	if (bestDistance > epsilon)
	{
		RefineSegment(edges, startIndex, bestIndex, epsilon, refinedIndices);
		refinedIndices.insert(bestIndex);
		RefineSegment(edges, bestIndex, endIndex, epsilon, refinedIndices);
	}
}

std::unordered_set<NavigationIndex> Manifest_Simulation::RamerDouglasPeucker(NavigationMesh& navigationMesh, NavigablePolygon& polygon)
{
	std::unordered_set<NavigationIndex> refinedIndices;
	LOG({ CONSOLE_BG_CYAN }, "before RDP:", polygon.cachedVertexToEdgeIndexMap.size(), "index:", polygon.polygonIndex);		
	//TODO: dynamic epsilon
	//measured in world units
	const MFfloat epsilon{ 15.0f };

	//generate initial polyline
	auto edges { navigationMesh.edges};
	const NavigationIndex startIndex{ polygon.beginEdgeIndex };	
	const NavigableEdge& start{ edges.at(startIndex) };
	NavigationIndex endIndex{ startIndex };
	NavigationIndex edgeIndex{ startIndex };
	MFfloat bestDistance{ 0.0f };
	do
	{
		const NavigableEdge& edge{ edges.at(edgeIndex) };
		const MFvec3 span{ edge.edgeOrigin - start.edgeOrigin };
		const MFfloat distance{ Dot(span,span) };
		if (distance > bestDistance)
		{
			bestDistance = distance;
			endIndex = edgeIndex;
		}
		edgeIndex = edge.nextIndex;
	} while (edgeIndex != startIndex);
	ASSERT(startIndex != endIndex);

	//start -> end -> start to close polygon 
	refinedIndices.insert(startIndex);	
	RefineSegment(navigationMesh.edges, startIndex, endIndex, epsilon, refinedIndices);
	refinedIndices.insert(endIndex);
	RefineSegment(navigationMesh.edges, endIndex, startIndex, epsilon, refinedIndices);	

	LOG({ CONSOLE_BG_YELLOW }, "after RDP:", refinedIndices.size(), "index:", polygon.polygonIndex);
	return refinedIndices;
}

void Manifest_Simulation::VoxelSpaceToWorldSpace(const MFtransform& voxelToWorld, NavigationMesh& navigationMesh)
{
	for (const NavigationIndex polygonIndex : navigationMesh.activePolygons)
	{
		NavigablePolygon& polygon{ navigationMesh.polygons.at(polygonIndex) };

		NavigationIndex edgeIndex{ polygon.beginEdgeIndex };
		do
		{
			NavigableEdge& edge{ navigationMesh.edges.at(edgeIndex) };
			edge.edgeOrigin = voxelToWorld * edge.edgeOrigin;
			edgeIndex = edge.nextIndex;
		} while (edgeIndex != polygon.beginEdgeIndex);
		polygon.surfacePlane = ComputeNavigablePlane(polygon, navigationMesh.edges);
		DLOG({ CONSOLE_BG_GREEN }, "polygon",polygon.polygonIndex,"surfacePlane:", polygon.surfacePlane);
	}
}

void Manifest_Simulation::SimplifyMesh(NavigationMesh& navigationMesh)
{ 
	for (const NavigationIndex polygonIndex : navigationMesh.activePolygons)
	{
		NavigablePolygon& polygon{ navigationMesh.polygons.at(polygonIndex) }; 
	
		const std::unordered_set<NavigationIndex> refinedIndices{ RamerDouglasPeucker(navigationMesh,polygon) };
		
		//build simplified polygon		
		if (refinedIndices.empty())
			return;
		polygon.beginEdgeIndex = *refinedIndices.begin();		
		polygon.cachedVertexToEdgeIndexMap.clear();
		NavigationIndex edgeIndex{ polygon.beginEdgeIndex };		
		do
		{
			NavigableEdge& edge{ navigationMesh.edges.at(edgeIndex) };

			//point was refined out - remove it from polygon
			if (!refinedIndices.contains(edge.edgeIndex))
			{
				NavigableEdge& incoming{ navigationMesh.edges.at(edge.previousIndex) };
				NavigableEdge& outgoing{ navigationMesh.edges.at(edge.nextIndex) };

				incoming.nextIndex = outgoing.edgeIndex;
				outgoing.previousIndex = incoming.edgeIndex;
				navigationMesh.freelist.deallocate(sizeof(edge), &edge);
			} 
			else
			{
				edge.edgeOrigin = ClosestPointOnPlane(polygon.surfacePlane, edge.edgeOrigin);
				edge.cachedVertex = CreateCachedVertex(edge.edgeOrigin);
				polygon.cachedVertexToEdgeIndexMap.insert(std::make_pair(edge.cachedVertex, edge.edgeIndex));
				LOG({ CONSOLE_BG_CYAN }, "keeping refined point:", edge.edgeIndex, "vertex:", edge.edgeOrigin);
			}
			edgeIndex = edge.nextIndex;
		} while (edgeIndex != polygon.beginEdgeIndex);
		polygon.vertexCount = refinedIndices.size();
	}
}
 
const MFplane Manifest_Simulation::ComputeNavigablePlane(const NavigablePolygon& navigablePolygon, std::vector<NavigableEdge>& edges)
{
	//project current points onto current plane
	MFvec3 normal{ 0 };
	//DLOG({ CONSOLE_BG_GREEN}, "polygon:", navigablePolygon.polygonIndex);
	NavigationIndex edgeIndex{ navigablePolygon.beginEdgeIndex };
	do
	{
		const NavigableEdge& edge{ edges.at(edgeIndex) };
		const MFpoint3& currentVertex{ edge.edgeOrigin};
		const MFpoint3& nextVertex{ edges.at(edge.nextIndex).edgeOrigin };
		normal.x += (currentVertex.y - nextVertex.y) * (currentVertex.z + nextVertex.z);
		normal.y += (currentVertex.z - nextVertex.z) * (currentVertex.x + nextVertex.x);
		normal.z += (currentVertex.x - nextVertex.x) * (currentVertex.y + nextVertex.y);
		//DLOG({ CONSOLE_BG_CYAN }, "edge:", edgeIndex,edge.edgeOrigin);
		edgeIndex = edges.at(edgeIndex).nextIndex;
	} while (edgeIndex != navigablePolygon.beginEdgeIndex);

	const MFfloat offset{ -Dot(normal,edges.at(edgeIndex).edgeOrigin) };

	return Normalize(MFplane{ normal,offset });
};

void Manifest_Simulation::ReserveBuffers(NavigationMesh& navigationMesh, MFu32 triangleCount)
{
	const MFsize vertexCount{ triangleCount };
	navigationMesh.edges.reserve(vertexCount);
	navigationMesh.polygons.reserve(vertexCount);	
}

void Manifest_Simulation::InsertTriangle(const MFtriangle& triangle, NavigationMesh& navigationMesh)
{
	std::array<CachedVertex, 3> cachedVertices
	{
		CreateCachedVertex(triangle.vertices[0]),
		CreateCachedVertex(triangle.vertices[1]),
		CreateCachedVertex(triangle.vertices[2])
	};

	//determine vertex combations per polygon	
	struct PolygonCode
	{
		NavigationIndex polygonIndex;
		NavigationIndex interiorIndex;
		MFu8 mergeCode;
		MFu8 interiorCode;
	};
	std::vector<PolygonCode> polygonCodes;
	for (MFu8 vertexIndex{ 0 }; vertexIndex < 3; ++vertexIndex)
	{
		MFu8 vertexCode{ 1u << vertexIndex };
		const CachedVertex cachedVertex{ cachedVertices.at(vertexIndex) };
		//DLOG({ CONSOLE_BG_MAGENTA }, "Checking for polygon cache hits with vertex:", cachedVertex.values[0], cachedVertex.values[1], cachedVertex.values[2], "[", triangle.vertices[vertexIndex], "]");
		for (const NavigationIndex polygonIndex : navigationMesh.activePolygons)
		{
			const NavigablePolygon& polygon{ navigationMesh.polygons.at(polygonIndex) };

			if (polygon.cachedVertexToInteriorIndexMap.contains(cachedVertex))
			{
				const NavigableEdge& interiorEdge{ navigationMesh.edges.at(polygon.cachedVertexToInteriorIndexMap.at(cachedVertex)) };
				const NavigablePolygon& interiorPolygon{ navigationMesh.polygons.at(interiorEdge.polygonIndex) };
				decltype(polygonCodes.begin()) codeIterator{ std::ranges::find_if(polygonCodes,[&](const PolygonCode polygonCode)->MFbool
			{
				return polygonCode.interiorIndex == interiorPolygon.polygonIndex;
			}) };
				if (polygonCodes.end() == codeIterator)
					polygonCodes.emplace_back(PolygonCode{ .polygonIndex {polygon.polygonIndex}, .interiorIndex{interiorPolygon.polygonIndex}, .mergeCode{0}, .interiorCode {vertexCode} });
				else
					codeIterator->interiorCode |= vertexCode;
			}

			if (!polygon.cachedVertexToEdgeIndexMap.contains(cachedVertex))
				continue;

			//DLOG({ CONSOLE_BG_YELLOW }, "Cache hit found on polygon:", polygonIndex, "with edge:", polygon.cachedVertexToEdgeIndexMap.at(cachedVertex));
			decltype(polygonCodes.begin()) codeIterator{ std::ranges::find_if(polygonCodes,[&](const PolygonCode polygonCode)->MFbool
			{
				return polygonCode.polygonIndex == polygon.polygonIndex;
			}) };
			if (polygonCodes.end() == codeIterator)
				polygonCodes.emplace_back(PolygonCode{ .polygonIndex {polygon.polygonIndex}, .interiorIndex {NO_INDEX}, .mergeCode { vertexCode},.interiorCode {0} });
			else
				codeIterator->mergeCode |= vertexCode;
		}		
	}
	//create new, potentially temporary, polygon for triangle	
	NavigablePolygon* newPolygon{ CreateNavigablePolygon(triangle, cachedVertices, navigationMesh) };

	for (const auto polygonCode : polygonCodes)
	{
		auto [polygonIndex, interiorIndex, mergeCode, interiorCode] { polygonCode };
		NavigablePolygon* oldPolygon{ &navigationMesh.polygons.at(polygonIndex) };
		NavigablePolygon* interiorPolygon{ nullptr };
		//PRIRORITIZE REDUCING AN INTERIOR POLYGON BEFORE EXPANDING EXTIOR
		//triangle fills a hole complete
		if (interiorCode == 7)
		{
			interiorPolygon = &navigationMesh.polygons.at(interiorIndex);
			return RemoveInteriorPolygon(cachedVertices, oldPolygon, interiorPolygon, newPolygon, navigationMesh);
		}

		switch (mergeCode)
		{
			//merge shared edge
			case 3://vertex 0 and 1 match 				
			case 5://vertex 2 and 0 match 				
			case 6://vertex 1 and 2 match				
				MergeSharedEdge(mergeCode, cachedVertices, oldPolygon, newPolygon, navigationMesh);				
				break;
				//special case 
			case 7://vertex 0 and 1 and 2 match
				FillMissingEdge(cachedVertices, oldPolygon, newPolygon, navigationMesh);
				break;
				//only one vertex match - reject potential convolution 
			case 1://vertex 0 match				
			case 2://vertex 1 match				
			case 4://vertex 2 match							
				break;
			default:
				break;
		}
	}
	//system("cls"); 
}

//im under the impression with how the triangles are inserted the only time a triangle will be inserted into an interior polygon is when it completely fills a temporary hole caused by insertertion ordering. other than this the XZ coordinates of the interior polygon shouldn't be visted again in a meaningful manner
void Manifest_Simulation::RemoveInteriorPolygon(std::array<CachedVertex, 3> cachedVertices, NavigablePolygon*& oldPolygon, NavigablePolygon*& interiorPolygon, NavigablePolygon*& newPolygon, NavigationMesh& navigationMesh)
{
	//remove interor edges 
	for (const CachedVertex cachedVertex : cachedVertices)
		oldPolygon->cachedVertexToInteriorIndexMap.erase(cachedVertex);
	//remove interior polygon 
	oldPolygon->interiorPolygons.erase(interiorPolygon->polygonIndex);	
	//deallocate
	NavigationIndex interiorIndex{ interiorPolygon->beginEdgeIndex };
	do
	{
		NavigableEdge& interiorEdge{ navigationMesh.edges.at(interiorIndex) };
		navigationMesh.freelist.deallocate(sizeof(interiorEdge), &interiorEdge);
		interiorIndex = interiorEdge.nextIndex;
	} while (interiorIndex != interiorPolygon->beginEdgeIndex);
	NavigationIndex newIndex{ newPolygon->beginEdgeIndex };
	do
	{
		NavigableEdge& newEdge{ navigationMesh.edges.at(newIndex) };
		navigationMesh.freelist.deallocate(sizeof(newEdge), &newEdge);
		newIndex = newEdge.nextIndex;
	} while (newIndex != newPolygon->beginEdgeIndex);
	interiorPolygon->cachedVertexToEdgeIndexMap.clear();
	newPolygon->cachedVertexToEdgeIndexMap.clear();
	navigationMesh.freelist.deallocate(sizeof(*interiorPolygon), interiorPolygon);	
	navigationMesh.freelist.deallocate(sizeof(*newPolygon), newPolygon);	
	navigationMesh.activePolygons.erase(newPolygon->polygonIndex); 
} 
 
void Manifest_Simulation::MergeSharedEdge(const MFu8 mergeCode, std::array<CachedVertex, 3> cachedVertices, NavigablePolygon*& oldPolygon, NavigablePolygon*& newPolygon, NavigationMesh& navigationMesh)
{  
	std::vector<NavigableEdge>& edges{ navigationMesh.edges };
	//PrintState(navigationMesh,*newPolygon);
	//PrintState(navigationMesh,*oldPolygon);

	//working		
	NavigableEdge* newIncoming{ nullptr };
	NavigableEdge* newOutgoing{ nullptr };
	NavigableEdge* newExpanding{ nullptr };
	NavigableEdge* oldIncoming{ nullptr };
	NavigableEdge* oldOutgoing{ nullptr };	
	switch (mergeCode)
	{				
		case 3://original insert vertex 2 excluded			
			newIncoming = &edges.at(newPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[1]));
			newOutgoing = &edges.at(newPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[0]));
			newExpanding = &edges.at(newPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[2]));
			oldIncoming = &edges.at(oldPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[1]));
			oldOutgoing = &edges.at(oldPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[0]));
			break;
		case 5://original insert vertex 1 excluded			
			newIncoming = &edges.at(newPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[0]));
			newOutgoing = &edges.at(newPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[2]));
			newExpanding = &edges.at(newPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[1]));
			oldIncoming = &edges.at(oldPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[0]));
			oldOutgoing = &edges.at(oldPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[2]));
			break;
		case 6://original insert vertex 0 excluded			
			newIncoming = &edges.at(newPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[2]));
			newOutgoing = &edges.at(newPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[1]));
			newExpanding = &edges.at(newPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[0]));
			oldIncoming = &edges.at(oldPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[2]));
			oldOutgoing = &edges.at(oldPolygon->cachedVertexToEdgeIndexMap.at(cachedVertices[1]));
			break;
		default:
			break;
	}
	ASSERT(newIncoming);	ASSERT(newOutgoing);	ASSERT(oldIncoming);	ASSERT(oldOutgoing);	
	//new polygon closes old polygon - split inner boundary from outter boundary
	if (oldOutgoing->previousIndex != oldIncoming->edgeIndex)
	{	
		//grab interior "alpha" and "beta" edges (joining in/out edges)
		NavigableEdge* interiorAlpha{ &edges.at(oldOutgoing->previousIndex) };
		NavigableEdge* interiorBeta{ &edges.at(oldIncoming->nextIndex) };


		//update interior pointers & cache
		interiorAlpha->nextIndex = newOutgoing->edgeIndex;
		newOutgoing->previousIndex = interiorAlpha->edgeIndex;
		interiorBeta->previousIndex = newIncoming->edgeIndex;
		newIncoming->nextIndex = interiorBeta->edgeIndex;
		newPolygon->cachedVertexToEdgeIndexMap.erase(newExpanding->cachedVertex);
		newPolygon->beginEdgeIndex = interiorAlpha->edgeIndex;
		NavigationIndex interiorIndex{ newPolygon->beginEdgeIndex };
		do
		{
			NavigableEdge& interiorEdge{ navigationMesh.edges.at(interiorIndex) };
			interiorEdge.polygonIndex = newPolygon->polygonIndex;
			const auto interiorCacheData{ std::make_pair(interiorEdge.cachedVertex, interiorEdge.edgeIndex) };
			if (!newPolygon->cachedVertexToEdgeIndexMap.contains(interiorEdge.cachedVertex))
				newPolygon->cachedVertexToEdgeIndexMap.insert(interiorCacheData);
			else
				newPolygon->cachedVertexToEdgeIndexMap.at(interiorEdge.cachedVertex) = interiorEdge.edgeIndex;
			oldPolygon->cachedVertexToInteriorIndexMap.insert(interiorCacheData);
			interiorIndex = interiorEdge.nextIndex;
		} while (interiorIndex != newPolygon->beginEdgeIndex);
		ConfirmState(navigationMesh, *newPolygon);

		//update exterior pointers & cache
		newExpanding->polygonIndex = oldPolygon->polygonIndex;
		newExpanding->nextIndex = oldOutgoing->edgeIndex;
		newExpanding->previousIndex = oldIncoming->edgeIndex;
		oldIncoming->nextIndex = newExpanding->edgeIndex;
		oldOutgoing->previousIndex = newExpanding->edgeIndex;		
		oldPolygon->beginEdgeIndex = newExpanding->edgeIndex;
		//probably a better way but not an easier way
		oldPolygon->cachedVertexToEdgeIndexMap.clear();
		NavigationIndex exteriorIndex{ oldPolygon->beginEdgeIndex };
		do
		{
			NavigableEdge& exteriorEdge{ navigationMesh.edges.at(exteriorIndex) }; 
			oldPolygon->cachedVertexToEdgeIndexMap.insert(std::make_pair(exteriorEdge.cachedVertex, exteriorEdge.edgeIndex));
			exteriorIndex = exteriorEdge.nextIndex;
		} while (exteriorIndex != oldPolygon->beginEdgeIndex);		
		ConfirmState(navigationMesh, *oldPolygon);
		oldPolygon->interiorPolygons.insert(newPolygon->polygonIndex);
		navigationMesh.activePolygons.erase(newPolygon->polygonIndex);
		

		return;
	}

	//keep the old polygons outgoing in favor of the new polygon
	//replace the old polygons incoming with the new polygon
	//DLOG({ CONSOLE_BG_CYAN }, "UPDATING:", newOutgoing->previousIndex, "nextIndex->",oldOutgoing->edgeIndex);
	//DLOG({ CONSOLE_BG_CYAN }, "UPDATING:", oldIncoming->previousIndex, "nextIndex->", newIncoming->edgeIndex);
	//DLOG({ CONSOLE_BG_CYAN }, "UPDATING:", newIncoming->edgeIndex, "nextIndex->", oldIncoming->previousIndex);
	//DLOG({ CONSOLE_BG_CYAN }, "UPDATING:", oldOutgoing->edgeIndex, "nextIndex->", newOutgoing->previousIndex);	
	newIncoming->previousIndex = oldIncoming->previousIndex;
	oldOutgoing->previousIndex = newOutgoing->previousIndex;
	edges.at(newOutgoing->previousIndex).nextIndex = oldOutgoing->edgeIndex; 
	edges.at(oldIncoming->previousIndex).nextIndex = newIncoming->edgeIndex;	

	//clean up tracking data	
	navigationMesh.activePolygons.erase(newPolygon->polygonIndex);
	newPolygon->cachedVertexToEdgeIndexMap.clear();	
	oldPolygon->cachedVertexToEdgeIndexMap.at(oldIncoming->cachedVertex) = newIncoming->edgeIndex;//replace incoming	
	//avoid invalidation
	oldPolygon->beginEdgeIndex = oldOutgoing->edgeIndex;	
	NavigationIndex edgeIndex{ oldPolygon->beginEdgeIndex };
	do
	{
		NavigableEdge& edge{ navigationMesh.edges.at(edgeIndex) };
		auto next{ edges.at(edge.nextIndex) };		
		edge.polygonIndex = oldPolygon->polygonIndex;
		if (!oldPolygon->cachedVertexToEdgeIndexMap.contains(edge.cachedVertex))
			oldPolygon->cachedVertexToEdgeIndexMap.insert(std::make_pair(edge.cachedVertex, edge.edgeIndex));
		else
			oldPolygon->cachedVertexToEdgeIndexMap.at(edge.cachedVertex) = edge.edgeIndex;
		edgeIndex = edge.nextIndex;
	} while (edgeIndex != oldPolygon->beginEdgeIndex);
	ConfirmState(navigationMesh, *oldPolygon);
	//deallocate
	navigationMesh.freelist.deallocate(sizeof(*newOutgoing), newOutgoing);
	navigationMesh.freelist.deallocate(sizeof(*oldIncoming), oldIncoming);
	navigationMesh.freelist.deallocate(sizeof(*newPolygon), newPolygon);	
	//update old polygon surface plane
	oldPolygon->surfacePlane = ComputeNavigablePlane(*oldPolygon, navigationMesh.edges);
	//absorb new polygon into new polygon
	std::ranges::move(newPolygon->interiorPolygons, std::inserter(oldPolygon->interiorPolygons, oldPolygon->interiorPolygons.begin()));
	std::ranges::move(newPolygon->cachedVertexToInteriorIndexMap, std::inserter(oldPolygon->cachedVertexToInteriorIndexMap, oldPolygon->cachedVertexToInteriorIndexMap.begin()));
	newPolygon = oldPolygon;	 
}

void Manifest_Simulation::FillMissingEdge(const std::array<CachedVertex, 3> cachedVertices, NavigablePolygon*& oldPolygon, NavigablePolygon*& newPolygon, NavigationMesh& navigationMesh)
{		
	
	ConfirmState(navigationMesh, *newPolygon);
	ConfirmState(navigationMesh, *oldPolygon);

	auto& edges{ navigationMesh.edges };
	NavigableEdge* newIncoming{ nullptr };
	NavigableEdge* newOutgoing{ nullptr };		
	NavigableEdge* oldIncoming{ nullptr };
	NavigableEdge* oldOutgoing{ nullptr };	
	NavigableEdge* oldRedundant{ nullptr };

	//detect edge with next vertex that doesn't cache to a vetex in the array	 
	for (const CachedVertex cachedVertex : cachedVertices)
	{ 
		const NavigationIndex cachedEdgeIndex{ oldPolygon->cachedVertexToEdgeIndexMap.at(cachedVertex)};
		NavigableEdge& cachedEdge{ navigationMesh.edges.at(cachedEdgeIndex) };
		NavigableEdge& nextEdge{ navigationMesh.edges.at(cachedEdge.nextIndex) };
		if (std::ranges::find(cachedVertices, nextEdge.cachedVertex) == cachedVertices.end())//cached edge is the old outgoing			
		{
			oldOutgoing = &cachedEdge;
			break;
		}
	}

	ASSERT(oldOutgoing);
	newOutgoing = &edges.at(newPolygon->cachedVertexToEdgeIndexMap.at(oldOutgoing->cachedVertex));
	newIncoming = &edges.at(newOutgoing->previousIndex);
	oldIncoming = &edges.at(oldPolygon->cachedVertexToEdgeIndexMap.at(newIncoming->cachedVertex));
	oldRedundant = &edges.at(oldIncoming->nextIndex);
	ASSERT(newIncoming);	ASSERT(newOutgoing);	ASSERT(oldIncoming);
	
	if (oldRedundant->nextIndex != oldOutgoing->edgeIndex)
	{
		for (MFu8 vertexIndex{ 0 }; vertexIndex < 3; ++vertexIndex)
		{			
			const CachedVertex cachedVertex{ cachedVertices.at(vertexIndex) };
			const NavigationIndex cachedEdgeIndex{ oldPolygon->cachedVertexToEdgeIndexMap.at(cachedVertex) };
			NavigableEdge& sharedOldEdge{ navigationMesh.edges.at(cachedEdgeIndex) };	
			NavigableEdge& sharedOldPrevious{ navigationMesh.edges.at(sharedOldEdge.previousIndex) };
			//look for shared edge(will be edge who's previous exists in cached vertices)
			if (std::ranges::find(cachedVertices, sharedOldPrevious.cachedVertex) == cachedVertices.end())
				continue;
			//this seems very finicky but we'll make more generic when i have an example that it fails
			NavigableEdge& sharedNewEdge{ navigationMesh.edges.at(newPolygon->cachedVertexToEdgeIndexMap.at(cachedVertex)) };
			NavigableEdge& closingNewEdge{ navigationMesh.edges.at(sharedNewEdge.previousIndex) };
			NavigableEdge& closingOldEdge{ navigationMesh.edges.at(oldPolygon->cachedVertexToEdgeIndexMap.at(closingNewEdge.cachedVertex)) };


			//update interior pointers & cache
			sharedOldPrevious.nextIndex = closingNewEdge.edgeIndex;
			closingNewEdge.previousIndex = sharedOldPrevious.edgeIndex;
			closingNewEdge.nextIndex = closingOldEdge.nextIndex;
			edges.at(closingOldEdge.nextIndex).previousIndex = closingNewEdge.edgeIndex;
			//old/new have same cached vertex
			newPolygon->cachedVertexToEdgeIndexMap.erase(sharedOldPrevious.cachedVertex);
			newPolygon->cachedVertexToEdgeIndexMap.erase(sharedNewEdge.cachedVertex);
			newPolygon->beginEdgeIndex = closingNewEdge.edgeIndex;
			NavigationIndex interiorIndex{ newPolygon->beginEdgeIndex };
			do
			{
				NavigableEdge& interiorEdge{ navigationMesh.edges.at(interiorIndex) };
				interiorEdge.polygonIndex = newPolygon->polygonIndex;
				const auto interiorCacheData{ std::make_pair(interiorEdge.cachedVertex, interiorEdge.edgeIndex) };
				if (!newPolygon->cachedVertexToEdgeIndexMap.contains(interiorEdge.cachedVertex))
					newPolygon->cachedVertexToEdgeIndexMap.insert(interiorCacheData);
				else
					newPolygon->cachedVertexToEdgeIndexMap.at(interiorEdge.cachedVertex) = interiorEdge.edgeIndex;
				oldPolygon->cachedVertexToInteriorIndexMap.insert(interiorCacheData);
				interiorIndex = interiorEdge.nextIndex;
			} while (interiorIndex != newPolygon->beginEdgeIndex);
			ConfirmState(navigationMesh, *newPolygon);

			//update exterior pointers & cache
			closingOldEdge.nextIndex = sharedOldEdge.edgeIndex;
			sharedOldEdge.previousIndex = closingOldEdge.edgeIndex;
			oldPolygon->cachedVertexToEdgeIndexMap.clear();
			oldPolygon->beginEdgeIndex = closingOldEdge.edgeIndex;
			NavigationIndex exteriorIndex{ oldPolygon->beginEdgeIndex };
			do
			{
				NavigableEdge& exteriorEdge{ navigationMesh.edges.at(exteriorIndex) };
				oldPolygon->cachedVertexToEdgeIndexMap.insert(std::make_pair(exteriorEdge.cachedVertex, exteriorEdge.edgeIndex));
				exteriorIndex = exteriorEdge.nextIndex;
			} while (exteriorIndex != oldPolygon->beginEdgeIndex);
			oldPolygon->interiorPolygons.insert(newPolygon->polygonIndex);
			navigationMesh.activePolygons.erase(newPolygon->polygonIndex);
			ConfirmState(navigationMesh, *oldPolygon);

			int y = 4;
			return; 
		}
	}
	ASSERT(oldRedundant->nextIndex == oldOutgoing->edgeIndex);	
	ASSERT(oldRedundant->previousIndex == oldIncoming->edgeIndex);
	ASSERT(oldOutgoing->previousIndex == oldRedundant->edgeIndex);
	ASSERT(oldIncoming->nextIndex == oldRedundant->edgeIndex);
	//strect old incoming to reach old outgoing
 	oldIncoming->nextIndex = oldOutgoing->edgeIndex;
	oldOutgoing->previousIndex = oldIncoming->edgeIndex;
	//clean up tracking data 
	//nav mesh first
	navigationMesh.activePolygons.erase(newPolygon->polygonIndex);		
	//polygon second
	newPolygon->cachedVertexToEdgeIndexMap.clear();		
	oldPolygon->cachedVertexToEdgeIndexMap.erase(oldRedundant->cachedVertex);
	
	//avoid invalidation
	oldPolygon->beginEdgeIndex = oldOutgoing->edgeIndex;	
	NavigationIndex edgeIndex{ oldPolygon->beginEdgeIndex };
	do
	{
		NavigableEdge& edge{ navigationMesh.edges.at(edgeIndex) };

		auto next{ edges.at(edge.nextIndex) };
		ASSERT(next.previousIndex == edge.edgeIndex);
		edge.polygonIndex = oldPolygon->polygonIndex;
		if (!oldPolygon->cachedVertexToEdgeIndexMap.contains(edge.cachedVertex))
			oldPolygon->cachedVertexToEdgeIndexMap.insert(std::make_pair(edge.cachedVertex, edge.edgeIndex));
		else
			oldPolygon->cachedVertexToEdgeIndexMap.at(edge.cachedVertex) = edge.edgeIndex;
		edgeIndex = edge.nextIndex;
	} while (edgeIndex != oldPolygon->beginEdgeIndex);
	ConfirmState(navigationMesh, *oldPolygon);
	//deallocate third
	edgeIndex = newPolygon->beginEdgeIndex;
	do
	{
		NavigableEdge& edge{ navigationMesh.edges.at(edgeIndex) };
		navigationMesh.freelist.deallocate(sizeof(edge), &edge);
		edgeIndex = edge.nextIndex;
	} while (edgeIndex != newPolygon->beginEdgeIndex);
	navigationMesh.freelist.deallocate(sizeof(*oldRedundant), oldRedundant);
	navigationMesh.freelist.deallocate(sizeof(*newPolygon), newPolygon);

	//update old polygon surface plane
	oldPolygon->surfacePlane = ComputeNavigablePlane(*oldPolygon, navigationMesh.edges);
	//absorb new polygon into new polygon
	newPolygon = oldPolygon; 
}

NavigablePolygon* Manifest_Simulation::CreateNavigablePolygon(const MFtriangle& triangle, const std::array<CachedVertex, 3>& cachedVertices, NavigationMesh& navigationMesh)
{
	//create new polygon
	void* polygonAdress{ navigationMesh.freelist.allocate(sizeof(NavigablePolygon)) };
	NavigablePolygon* newPolygon{ nullptr };
	if (polygonAdress == FreeList_NEW::NO_FREELIST_ENTRY)
		newPolygon = &navigationMesh.polygons.emplace_back();
	else
		newPolygon = new(polygonAdress)NavigablePolygon;
	//calculate new polygon index
	newPolygon->polygonIndex = std::distance(navigationMesh.polygons.data(), newPolygon);
	navigationMesh.activePolygons.insert(newPolygon->polygonIndex);	
	//create polygon edges - MUST HAVE ENOUGH ROOM IN BUFFER TO AVOID RESIZE
	void* edg0Address{ navigationMesh.freelist.allocate(sizeof(NavigableEdge)) };
	NavigableEdge* edge0{ nullptr };
	if (edg0Address == FreeList_NEW::NO_FREELIST_ENTRY)
		edge0 = &navigationMesh.edges.emplace_back();
	else
		edge0 = new(edg0Address)NavigableEdge;

	void* edge1Address{ navigationMesh.freelist.allocate(sizeof(NavigableEdge)) };
	NavigableEdge* edge1{ nullptr };
	if (edge1Address == FreeList_NEW::NO_FREELIST_ENTRY)
		edge1 = &navigationMesh.edges.emplace_back();
	else
		edge1 = new(edge1Address)NavigableEdge;

	void* edg2Address{ navigationMesh.freelist.allocate(sizeof(NavigableEdge)) };
	NavigableEdge* edge2{ nullptr };
	if (edg2Address == FreeList_NEW::NO_FREELIST_ENTRY)
		edge2 = &navigationMesh.edges.emplace_back();
	else
		edge2 = new(edg2Address)NavigableEdge;

	NavigableEdge* const edgeData{ navigationMesh.edges.data() };
	edge0->edgeIndex = std::distance(edgeData, edge0);
	edge1->edgeIndex = std::distance(edgeData, edge1);
	edge2->edgeIndex = std::distance(edgeData, edge2);
	edge0->edgeOrigin = triangle.vertices[0];
	edge1->edgeOrigin = triangle.vertices[1];
	edge2->edgeOrigin = triangle.vertices[2];
	edge0->cachedVertex = cachedVertices[0];
	edge1->cachedVertex = cachedVertices[1];
	edge2->cachedVertex = cachedVertices[2];
	edge0->nextIndex = edge1->edgeIndex;
	edge1->nextIndex = edge2->edgeIndex;
	edge2->nextIndex = edge0->edgeIndex;
	edge0->previousIndex = edge2->edgeIndex;
	edge1->previousIndex = edge0->edgeIndex;
	edge2->previousIndex = edge1->edgeIndex;
	edge0->polygonIndex = edge1->polygonIndex = edge2->polygonIndex = newPolygon->polygonIndex;	
	newPolygon->polygonIndex = newPolygon->polygonIndex;
	newPolygon->beginEdgeIndex = edge0->edgeIndex;
	newPolygon->surfacePlane = ComputeNavigablePlane(*newPolygon, navigationMesh.edges);
	//cache vertices and edges, update existing buffers if vertex exists 
	std::array<NavigationIndex, 3> edgeIndices{ edge0->edgeIndex,edge1->edgeIndex,edge2->edgeIndex };
	std::ranges::for_each(cachedVertices, [&, vertexIndex = 0](const CachedVertex& cachedVertex) mutable ->void
	{
		newPolygon->cachedVertexToEdgeIndexMap[cachedVertices[vertexIndex]] = edgeIndices.at(vertexIndex);
		vertexIndex++;
	}); 

	return newPolygon;
};

 
NavigablePolygon const *  Manifest_Simulation::ClosestPolygonToPoint(const NavigationMesh& navigationMesh, const MFpoint3& point)
{
	NavigablePolygon const* result{ nullptr };

	for (NavigationIndex polygonIndex : navigationMesh.activePolygons)
	{
		const NavigablePolygon& polygon{ navigationMesh.polygons.at(polygonIndex) };
		//check polygonal surface itself - this is so bad please for the love of god
		if (Dot(polygon.surfacePlane, point) >= 5.0f)
		{
			continue;
		}		
		//create side plane containing each polygon edge and check that point is behind it
		MFbool containsPoint{ true };
		NavigationIndex edgeIndex{ polygon.beginEdgeIndex };
		do
		{
			const NavigableEdge& edge{ navigationMesh.edges.at(edgeIndex) };
			const NavigableEdge& next{ navigationMesh.edges.at(edge.nextIndex) };
			//vector along side plane containing polygon edge
			const MFvec3 faceVector{ Normalize(next.edgeOrigin - edge.edgeOrigin) };			
			const MFvec3 normal{ Cross(polygon.surfacePlane.Normal(),faceVector)};
			const MFfloat offset{ -Dot(normal,edge.edgeOrigin) };
			//check point is behind plane
			const MFplane sidePlane{ Normalize(MFplane{normal,offset}) };
			const MFfloat distance{ Dot(sidePlane,point) };
			
			edgeIndex = next.edgeIndex;
			if (distance <-0.1f)
			{				
				containsPoint = false;
				break;				
			}			
		} while (edgeIndex!= polygon.beginEdgeIndex);
		if (containsPoint)
		{
			result = &polygon;
			break;
		}				
	}

	return result;
}

void Manifest_Simulation::PrintState(const NavigationMesh& navigationMesh, const NavigablePolygon& navigablePolygon)
{	
	//confirm invariant state
	NavigationIndex edgeIndex{ navigablePolygon.beginEdgeIndex };
	do
	{
		const NavigableEdge& edge{ navigationMesh.edges.at(edgeIndex) };
		LOG({ CONSOLE_BG_RED }, "EDGE:", edgeIndex, "ORIGIN:", edge.edgeOrigin);
		edgeIndex = edge.nextIndex;
	} while (edgeIndex != navigablePolygon.beginEdgeIndex);
}

MFbool Manifest_Simulation::ConfirmState(const NavigationMesh& navigationMesh, const NavigablePolygon& navigablePolygon)
{
	return true;

	//DLOG({ CONSOLE_BG_BLUE }, "CONFIRMING STATE OF POLYGON:", navigablePolygon.polygonIndex);
	//confirm invariant state
	NavigationIndex edgeIndex{ navigablePolygon.beginEdgeIndex };
	do
	{
		const NavigableEdge& edge{ navigationMesh.edges.at(edgeIndex) };
		ASSERT(edge.polygonIndex == navigablePolygon.polygonIndex);
		//DLOG({ CONSOLE_BG_RED }, "CHECKING EDGE:", edgeIndex, "ORIGIN:",edge.edgeOrigin, "FOR INVARIANT STATE P:", edge.previousIndex, "N:", edge.nextIndex);
		const auto next{ navigationMesh.edges.at(edge.nextIndex) };
		ASSERT(next.previousIndex == edge.edgeIndex);
		const auto previous{ navigationMesh.edges.at(edge.previousIndex) };
		ASSERT(previous.nextIndex == edge.edgeIndex);
		edgeIndex = edge.nextIndex;
	} while (edgeIndex != navigablePolygon.beginEdgeIndex);

	return true;
}