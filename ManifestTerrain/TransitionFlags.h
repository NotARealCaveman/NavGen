#pragma once

#include <ManifestUtility/Typenames.h>
#include "TerrainFlags.h"

using namespace Manifest_Utility;

namespace Manifest_Terrain
{
    using SharedCornerData = MFu8;

    //using the faceMask variable as an index get the bit code for the state of the shared vertex
  //each axis is a 2 bit code read as ZX with the leading bit of x as bit 0 (b3 b2 b1 b0)
  //the 2 bits describe wether the vertex is in a shared maximal/minimal state, and wether the face is positive or negative., ex (Mz),-Z,(mx),+X: 1 0 0 1 : 9   
   //these codes are used for determining which corners need to be manipulated in the case of a transition    
    const SharedCornerData CornerTable[NUMBER_TRANSITION_FACES]
    {
        0x00,//return 0    0000
        0x03,//return 3    0011
        0x0F,//return 15   1111
        0x0C,//return 12   1100                      
    };
    //holds the number of transition corners and the codes for their state
    struct TransitionCombination
    {
        const MFu8 transitionCount;
        const SharedCornerData cornerCombinations[NUMBER_TRANSITION_FACES];
    };
    //holds the possible transition corner combinations
    const TransitionCombination TransitionCombinationTable[16]
    {
        {0,{}},//0
        {0,{}},//1
        {0,{}},//2
        {1,{CornerTable[0]}},//3,0
        {0,{}},//4
        {0,{}},//5
        {1,{CornerTable[1]}},//6,1
        {2,{CornerTable[0],CornerTable[1]}},//7,0&1
        {0,{}},//8
        {1,{CornerTable[3]}},//9,3
        {0,{}},//A
        {2,{CornerTable[3],CornerTable[0]}},//B,3&0
        {1,{CornerTable[2]}},//C,2
        {2,{CornerTable[2],CornerTable[3]}},//D,2&3
        {2,{CornerTable[1],CornerTable[2]}},//E,1&2
        {4,{CornerTable[0],CornerTable[1],CornerTable[2],CornerTable[3]}},//F,0-3?not possible
    };
    //helper struct to avoid errors when using corner codes
    struct TransitionData
    {
        MFu8& xFace;
        MFu8& xExtreme;
        MFu8& zFace;
        MFu8& zExtreme;
    };

    //returns the extraction indesx based on config        
    inline MFu32 PrimaryExtractionDataIndex(const MFu32 chunkIndex, const MFu32 chunksPerLOD, const MFu8 chunkLOD)
    {
        return  chunkIndex + chunksPerLOD * chunkLOD;
    }
    inline MFu32 TransitionExtractionDataIndex(const MFu32 chunkIndex, const MFu32 chunksPerLOD, const MFu8 chunkLOD)
    {
        return  chunkIndex + chunksPerLOD * (chunkLOD-1);
    }
    
    /*
    //return the transition terrain index based on config
    inline MFu32 TransitionTerrainIndex(const MFu32 chunkIndex, const MFu8 chunkLOD, const MFu8 transitionFace)
    {
        return transitionFace + (chunkIndex*NUMBER_TRANSITION_FACES);
    }*/
    
    //if bit 0 is on dealing with +/- z axis
    inline MFu8 TransitionCoord(const MFu8& face) { return face & 0x01 ? 2 : 0; };
    //if bit 0 is on dealing with +/- z axis
    inline MFu8 ComplimentaryCoord(const MFu8& face) { return face & 0x01 ? 0 : 2; };
    //bit 0 is the +/- x face, bit 1 is the min/max x copmliment, bit 2 is the +/- z face, bit 3 is the min/max z compliment
    void ExtractTransitionData(const SharedCornerData& sharedCornerData, TransitionData&& transitionData);
    //calculates the amount of room required for the transition meshes
    MFdouble CalculateAxisDelta(const MFint32 voxelSample, const MFu8 chunkLOD, const MFu8 worldLOD);
}
