#include "Clipping.h"

using namespace Manifest_Math;

void Manifest_Math::ClipSegments(const MFpoint3(&segment0)[2],  MFpoint3(&segment1)[2])
{
	//calculate clipping planes of first segment
	const MFvec3 normal0{ segment0[0] - segment0[1] };	
	const MFplane plane0{ Normalize(MFplane{ normal0,-Dot(normal0,segment0[0])}) };
	const MFvec3 normal1{ segment0[1] - segment0[0] };
	const MFplane plane1{ Normalize(MFplane{ normal1,-Dot(normal1,segment0[1])}) };
	;// DLOG({ CONSOLE_DEFAULT }, "plane0", plane0, "plane1", plane1);
	//clip vertices of second segment against positive clipping planes
	for (MFu32 vertex{ 0 }; vertex < 2; ++vertex)
	{
		;// DLOG({ CONSOLE_BLUE }, "Clipping vertex:", segment1[vertex]);
		MFfloat projection0{ Dot(plane0, segment1[vertex]) };
		if (Dot(plane0, segment1[vertex]) > 0.0f)
			segment1[vertex] = ClosestPointOnPlane(plane0, segment1[vertex]);
		MFfloat projection1{ Dot(plane1, segment1[vertex]) };
		if (Dot(plane1, segment1[vertex]) > 0.0f)
			segment1[vertex] = ClosestPointOnPlane(plane1, segment1[vertex]);		
	}
}