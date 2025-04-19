#pragma once
#include "MarchingCubes.h"

namespace Manifest_Terrain
{
	//THIS TRANSVOXEL IMPLEMENTATION IS A 2D GENERALIZATION
	//THERE ARE NO VOXEL SPACE Z TRANSITIONS ALLOWED
	//SEE MARCHINGCUBES.H FOR SIZE DETAILS
		
	//transition cell indexing - indices are given via relation to the hex code but correspond to distance array
	//40----20----10//6-7-8				Z
	//|		|	  |						|  Y
	//|		|	  |						| /
	//80----100---08//3-4-5				|/_ _ _X
	//|		|	  | 
	//|		|	  |
	//01----02----04//0-1-2

	struct TransitionCellStorage
	{
		MFu16 reuseVertex[10];
	};

	//bit 4 indicates wether the face is the low resolution face if on
	//bits 0-3 indicate the planar offset in the high/low resolution face
	//the 5 bit offset code is set by N b1 b0 a1 a0 where b and a are the axes spanned by the planar transition face whose normal N is the remaining axis
	//bits a0/a1 and b0/b1 are never set together. 2 bits are used to indicate wether the axis needs a full step of a half step from the transition LOD
	//the table entries can be indexed by the corresponding voxel sample index
	const MFu8 TransitionOffsetTable[13] =
	{
		0x00,0x01,0x02,	//high res lower
		0x04,0x05,0x06,	//high res middle
		0x08,0x09,0x0A,	//high res upper
		0x10,0x12,		//low res lower
		0x18,0x1A		//high res upper
	};

	//generalized extraction methods
	void ExtractTransitionTerrainData(const Voxel* field, const MFint32 n, const MFint32 m, const MFu8 extractionLOD, const MFu8 worldLOD, const Integer3D& minBlockCorner, TransitionTerrain(&transitionTerrainArray)[4]);
	void ProcessTransitionCell_LOD(const Voxel* field, const MFint32& nField, const MFint32 mField, const MFint32 i, const MFint32 j, const MFint32 k, TransitionCellStorage* const(&rowStorage)[2], const MFu32 deltaMask, const MFu8 extractionLOD, const MFu8 axis, const MFu8 worldLOD, const Integer3D& minBlockCorner, TransitionTerrain& transitionTerrain);
	//specialized extraction methods		
	MFu32 LoadTransitionCell_X(const Voxel* field, const MFint32& n, const MFint32& m, const MFint32& i, const MFint32& j, const MFint32& k, Voxel* distance, const MFu8& lodScale);
	MFu32 LoadTransitionCell_Y(const Voxel* field, const MFint32& n, const MFint32& m, const MFint32& i, const MFint32& j, const MFint32& k, Voxel* distance, const MFu8& lodScale);
	Integer3D CalculateOffset_X(const MFu8& corner, const MFu8& lod);
	Integer3D CalculateOffset_Y(const MFu8& corner, const MFu8& lod);
	//i,j,k have been generalized down to a and b where the normal of the plane spanned by them is the third axis
    inline MFu16* ReuseTransitionVertex(const MFint32& a, TransitionCellStorage* const(&rowStorage)[2], MFu16 reuseCode, const MFu16& deltaCode)
    {		
		MFint32 da = deltaCode & 1;
		MFint32 db = deltaCode>>1;
		//rowStorage[1] is the preceeding row
		TransitionCellStorage* row = rowStorage[db];
		//return the vertex index store in the preceding cell. the new corner index can never be zero
		return &row[a - da].reuseVertex[reuseCode];
    }
	
}