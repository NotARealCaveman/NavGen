#pragma once
#include <map>

#include <ManifestMath/Transform.h  >
#include <ManifestMath/Point3.h>
#include <ManifestMath/Integer3D.h>
#include <ManifestMath/Grid2D.h>
#include <ManifestTerrain/Voxel.h>
#include <ManifestTerrain/TerrainFlags.h>

//https://transvoxel.org/Transvoxel.cpp
#include "Transvoxel_Lookup_ERIC_LENGYEL2009.h"//original source copied - 1/20/2022 ty good sir
#include "ExtractionData.h"
#include <ManifestTerrain/TransitionFlags.h>

//THIS FILE IMPLEMENTS THE MARCHING CUBE ALGORITHIM FOR AN ENTIRE ISOSURFACE
//ALSO SUPPORTS EXTRACTION FOR INDIVIDUAL BLOCKS OUT OF A VOXEL MAP
//ALSO SUPPORTS VARIOUS LODS
namespace Manifest_Terrain
{    
    //extracts a given portion of a voxel map at a given sampling resolution
    //extracted blocks always contain enough cells to span the width of the block at the highest lod possible. this simplifies the transition process as well as eases load off of GPU, Eric Lengyel 12/20 update
    //by removing the ability for a transition along the Up axis, the octree is no longer required    
    void ExtractPrimaryTerrainData(const Voxel* field, const Integer3D& minCorner, const MFint32 n, const MFint32 m, const MFu8 extractionLOD, const MFu8 worldLOD, PrimaryTerrain& primaryTerrain);
    void ProcessCell(const Voxel* field, const Integer3D& minCorner, const MFint32 nField, const MFint32 mField, const MFint32 nVoxels, const MFint32 mVoxels, const MFint32 i, const MFint32 j, const MFint32 k, CellStorage* const(&deckStorage)[2], const MFu32 deltaMask,const MFu8 lod, PrimaryTerrain& primaryTerrain);
    MFu32 LoadCell(const Voxel* field, const MFint32& n, const MFint32& m, const MFint32& i, const MFint32& j, const MFint32& k, Voxel* distance, const MFu8& lodScale = 1);
    //recursively step through sample points to find the sample match at the highest level of detail
    void SurfaceShift(const MFu8& currentLoD, Integer3D& minSample, Integer3D& maxSample, const Voxel* field, const MFu32& nField,const MFu32 mField, const MFu8(&corner)[2], Voxel& d0, Voxel& d1);
    //returns the surface normal of the triangle, which can be used for validation
    Integer3D ValidationNormal(const Integer3D& a, const Integer3D& b, const Integer3D& c, const Integer3D& minCorner);    

    inline MFu32* ReuseCornerVertex(const MFint32& n, const MFint32& i, const MFint32& j, CellStorage* const(&deckStorage)[2], MFu16 cornerIndex, const MFu16& deltaCode)
    {
        //the corner index in the preceeding cell is the sum of the original corner index and the masked delta code
        cornerIndex += deltaCode;
        //the three bits of the delta code indicate wether one should be subtracted from the cell coords in the x,y, and z directions
        MFint32 dx = deltaCode & 1;
        MFint32 dy = (deltaCode >> 1) & 1;//y code is 2 bits 
        MFint32 dz = deltaCode >> 2;
        //deckStorage[0] points to the current deck, deckStorage[1] points to the preceding deck
        CellStorage* deck = deckStorage[dz];
        //return the address of the vertex index in the preceeding cell. the new corner index can never be zero
        return (&deck[(j - dy) * n + (i - dx)].corner[cornerIndex - 1]);
    }

    inline MFu32 ReuseEdgeVertex(const MFint32& n, const MFint32& i, const MFint32& j, CellStorage* const(&deckStorage)[2], MFu16 edgeIndex, const MFu16& deltaCode)
    {
        //edge index in preceding cell differs from original index x by 3 for each of the lowest three bits in the masked delta code, and by 6 for the highest bit
        edgeIndex += ((deltaCode & 1) + ((deltaCode >> 1) & 1) + ((deltaCode >> 2) & 3)) * 3;
        //bits 0,1, and 3 of the delta code indicate wether one should be subtracted from the cell coords in the x,y, and z directions
        MFint32 dx = deltaCode & 1;
        MFint32 dy = (deltaCode >> 1) & 1;
        MFint32 dz = deltaCode >> 3;
        //deckStorage[0] is the current deck, deskStorage[1] is the preceeding deck
        const CellStorage* deck = deckStorage[dz];        
        //return the vertex index stored in the preceding cell the new edge index can never  be less than 3
        return deck[(j - dy) * n + (i - dx)].edge[edgeIndex - 3];
    }

    //returns the voxel value from a given SDF
    inline Voxel GetVoxel(const Voxel* field, const MFint32& n, const MFint32& m, const MFint32& i, const MFint32& j, const MFint32& k)
    {
        return field[i + n * (j + m * k)];
    }
}