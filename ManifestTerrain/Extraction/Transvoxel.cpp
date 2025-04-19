#include "Transvoxel.h"

using namespace Manifest_Terrain;

void Manifest_Terrain::ExtractTransitionTerrainData(const Voxel* field, const MFint32 n, const MFint32 m, const MFu8 extractionLOD, const MFu8 worldLOD, const Integer3D& minBlockCorner, TransitionTerrain(&transitionTerrain)[4])
{
    const MFint32 NUMBER_CELLS = BASE_CELLS_PER_BLOCK << (worldLOD - extractionLOD);
    TransitionCellStorage* rowStorage[2];
    //allocate storage for two rows of history - no decks as transitions are 1 cell 
    TransitionCellStorage* precedingCellStorage = new TransitionCellStorage[NUMBER_CELLS * 2];
    TransitionCellStorage* storageBegin = precedingCellStorage;
    MFu16 deltaMask = 0;            
    //loop minimum faces of block      -X[0]     
    for (MFint32 k = 0; k < NUMBER_CELLS; ++k)
    {
        rowStorage[0] = &precedingCellStorage[NUMBER_CELLS * (k & 1)];
        for (MFint32 j = 0; j < NUMBER_CELLS; ++j)
        {
            ProcessTransitionCell_LOD(field, n, m, 0, j, k, rowStorage, deltaMask, extractionLOD, worldLOD, 0, minBlockCorner, transitionTerrain[0]);
            deltaMask |= 1;//allow reuse in y direction                  
        }
        deltaMask = 2;//allow reuse in z direction
        rowStorage[1] = rowStorage[0];//current deck becomes preceding deck
    }        
    //reset storage/reuse           -Y/Z[1]
    memset(storageBegin, 0, sizeof(TransitionCellStorage) * NUMBER_CELLS * 2);    
    deltaMask = 0;
    for (MFint32 k = 0; k < NUMBER_CELLS; ++k)
    {
        rowStorage[0] = &precedingCellStorage[NUMBER_CELLS * (k & 1)];
        for (MFint32 i = 0; i < NUMBER_CELLS; ++i)
        {
            ProcessTransitionCell_LOD(field, n, m, i, 0, k, rowStorage, deltaMask, extractionLOD, worldLOD, 1,minBlockCorner, transitionTerrain[1]);
            deltaMask |= 1;//allow reuse in x direction                  
        }
        deltaMask = 2;//allow reuse in z direction
        rowStorage[1] = rowStorage[0];//current deck becomes preceding deck
    }        
    //loop maximum faces of block    +X[2]     
    memset(storageBegin, 0, sizeof(TransitionCellStorage) * NUMBER_CELLS * 2);    
    deltaMask = 0;
    for (MFint32 k = 0; k < NUMBER_CELLS; ++k)
    {
        rowStorage[0] = &precedingCellStorage[NUMBER_CELLS * (k & 1)];
        for (MFint32 j = 0; j < NUMBER_CELLS; ++j)
        {
            ProcessTransitionCell_LOD(field, n, m, NUMBER_CELLS, j, k, rowStorage, deltaMask, extractionLOD, worldLOD,0, minBlockCorner, transitionTerrain[2]);
            deltaMask |= 1;//allow reuse in y direction                  
        }
        deltaMask = 2;//allow reuse in z direction
        rowStorage[1] = rowStorage[0];//current deck becomes preceding deck
    }    
    memset(storageBegin, 0, sizeof(TransitionCellStorage) * NUMBER_CELLS * 2);    
    deltaMask = 0;//                  -Y/Z[3]
    for (MFint32 k = 0; k < NUMBER_CELLS; ++k)
    {
        rowStorage[0] = &precedingCellStorage[NUMBER_CELLS * (k & 1)];
        for (MFint32 i = 0; i < NUMBER_CELLS; ++i)
        {
           ProcessTransitionCell_LOD(field, n, m, i, NUMBER_CELLS, k, rowStorage, deltaMask, extractionLOD, worldLOD,1, minBlockCorner, transitionTerrain[3]);
            deltaMask |= 1;//allow reuse in x direction                  
        }
        deltaMask = 2;//allow reuse in z direction
        rowStorage[1] = rowStorage[0];//current deck becomes preceding deck
    }    
    
    delete[] precedingCellStorage;
}

void Manifest_Terrain::ProcessTransitionCell_LOD(const Voxel* field, const MFint32& nField, const MFint32 mField, const MFint32 i, const MFint32 j, const MFint32 k, TransitionCellStorage* const(&rowStorage)[2], const MFu32 deltaMask, const MFu8 extractionLOD, const MFu8 worldLOD, const MFu8 axis, const Integer3D& minBlockCorner, TransitionTerrain& transitionTerrain)
{
    auto& extractionData{ transitionTerrain.extractionData };
    auto& transitionHalfResolutionIndices{ transitionTerrain.halfResolutionVertexIndices };

    Voxel distance[13];
    //select storage based on axis
    TransitionCellStorage* cellStorage = axis == 0 ? &rowStorage[0][j] : &rowStorage[0][i];
    //set reuse vertex values for detection
    for (MFint32 a = 0; a < 10; ++a)
        cellStorage->reuseVertex[a] = 0xFFFF;
    //create base and select case index based on axis
    const Integer3D base{ minBlockCorner.i + (i << extractionLOD),minBlockCorner.j + (j << extractionLOD),minBlockCorner.k + (k << extractionLOD) };
    MFu32 caseIndex = axis == 0 ? LoadTransitionCell_X(field, nField, mField, base.i, base.j, base.k, distance, 1 << extractionLOD) : LoadTransitionCell_Y(field, nField, mField, base.i, base.j, base.k, distance, 1 << extractionLOD);
    //determine relevant transition data
    MFu32 equivalenceClass = transitionCellClass[caseIndex];
    const TransitionCellData* classData = &transitionCellData[equivalenceClass & 0x7F];
    MFbool inverseWinding = equivalenceClass & 0x80;
    MFu32 geometryCounts = classData->geometryCounts;
    if (geometryCounts)
    {
        MFu16 cellVertexIndex[12];
        MFint32 vertexCount = geometryCounts >> 4;
        MFint32 triangleCount = geometryCounts & 0x0F;
        //load case vertex code and begin creating vertices
        const MFu16* vertexCode = transitionVertexData[caseIndex];
        for (MFint32 a = 0; a < vertexCount; ++a)
        {
            MFu16 vertexIndex;
            MFu8 corner[2];
            Integer3D position[2];
            //calculate examined corners of current sample
            MFu16 vCode = vertexCode[a];
            corner[0] = (vCode >> 4) & 0x0F;
            corner[1] = vCode & 0x0F;
            //determine if this vertex is along the half resolution face
            const MFbool halfResolutionFace = corner[0] > 0x08 || corner[1] > 0x08;
            //calculate interpolation paramter
            Voxel d0 = distance[corner[0]];
            Voxel d1 = distance[corner[1]];
            MFint32 t = (d1 << 8) / (d1 - d0);            
            //calculate positions based on axis offsets
            if (axis == 0)
            {
                position[0] = base + CalculateOffset_X(corner[0], extractionLOD);
                position[1] = base + CalculateOffset_X(corner[1], extractionLOD);
            }
            else
            {
                position[0] = base + CalculateOffset_Y(corner[0], extractionLOD);
                position[1] = base + CalculateOffset_Y(corner[1], extractionLOD);
            }
            //place vertex at edge
            if (t)
            {//surface shift if required
                if (extractionLOD > 1 || halfResolutionFace)
                {
                    //determine midpoint sample                
                    SurfaceShift(extractionLOD, position[0], position[1], field, nField, mField, corner, d0, d1);
                    t = (d1 << 8) / (d1 - d0);
                }
                //vertex falls in the interior of an edge. extract edge index and delta code from vertex code
                //vcode is 16 bits low nibble of upper byte is the vertex index/reuse code
                MFu16 reuseData = (vCode >> 8);
                MFu16 reuseCode = reuseData & 0x0F;
                MFu16 reuseMask = reuseData >> 4;
                //bits 3&4 high nibble of upper byte indicates new vertex creation
                if (reuseMask & 0x0C)
                {
                    vertexIndex = extractionData.nVertices++;
                    Integer3D* vertex = &extractionData.vertices[vertexIndex];
                    *vertex = ((position[0]) * t + (position[1]) * (0x0100 - t));
                    //bit 4 indicates a reuseable new vertex
                    if (reuseMask & 0x08)                    
                        cellStorage->reuseVertex[reuseCode] = vertexIndex;                    
                    cellVertexIndex[a] = vertexIndex;
                    if (halfResolutionFace)
                        transitionHalfResolutionIndices.emplace_back(vertexIndex);
                    continue;
                }
                MFu16 deltaCode = reuseMask & deltaMask;
                if (deltaCode)
                    vertexIndex = axis == 0 ? *ReuseTransitionVertex(j, rowStorage, reuseCode, deltaCode) : *ReuseTransitionVertex(i, rowStorage, reuseCode, deltaCode);
                //generate a new vertex 
                else
                {
                    vertexIndex = extractionData.nVertices++;
                    Integer3D* vertex = &extractionData.vertices[vertexIndex];
                    *vertex = ((position[0]) * t + (position[1]) * (0x0100 - t));
                    //store vertex index for potential reuse
                    if (reuseMask & 0x80)
                        cellStorage->reuseVertex[reuseCode] = vertexIndex;                    
                }
                cellVertexIndex[a] = vertexIndex;
                if (halfResolutionFace)
                    transitionHalfResolutionIndices.emplace_back(vertexIndex);
            }
            else
            {
                //determine corner the vertex falls at
                MFu8 c = t == 0;
                MFu8 cornerIndex = corner[c];
                //corner vertex in precedin cell may not have been generated so we get address and look for valid index
                MFu16* indexAddress = nullptr;
                MFu8 cornerData = transitionCornerData[cornerIndex];
                MFu8 reuseCode = cornerData & 0x0F; 
                //bits 3&4 in high nibble forces vertex generation
                if (cornerData & 0xC0)
                {
                    vertexIndex = extractionData.nVertices++;
                    extractionData.vertices[vertexIndex] = position[c] << 8;
                    //bit 4 indicates a reuseable new vertex      
                    if (cornerData & 0x80)
                        cellStorage->reuseVertex[reuseCode] = vertexIndex;                    
                    cellVertexIndex[a] = vertexIndex;
                    if (halfResolutionFace)
                        transitionHalfResolutionIndices.emplace_back(vertexIndex);
                    continue;
                }
                MFu16 deltaCode = (cornerData >> 4) & deltaMask;
                //reuse corner vertex if possible
                if (deltaCode)
                    indexAddress = axis == 0 ? ReuseTransitionVertex(j, rowStorage, reuseCode, deltaCode) : ReuseTransitionVertex(i, rowStorage, reuseCode, deltaCode);
                vertexIndex = indexAddress ? *indexAddress : 0xFFFF;
                //reuse not generated or reuse not allowed
                if (vertexIndex == 0xFFFF)
                {
                    vertexIndex = extractionData.nVertices++;
                    //reuse allowed for this vertex
                    if (indexAddress)                    
                        *indexAddress = vertexIndex;                                        
                    extractionData.vertices[vertexIndex] = position[c] << 8;
                }
                cellVertexIndex[a] = vertexIndex;
                if (halfResolutionFace)
                    transitionHalfResolutionIndices.emplace_back(vertexIndex);
            }
        }
        //generate triangles for this cell using table data
        const MFu8* classVertexIndex = classData->vertexIndex;
        Triangle* meshTriangle = &extractionData.triangles[extractionData.nTriangles];
        extractionData.nTriangles += triangleCount;
        if ((i && axis == 0) || (!j && axis == 1))//swap inverse for min y and max x
            inverseWinding = !inverseWinding;
        if (inverseWinding)
        {
            for (MFint32 a = 0; a < triangleCount; ++a)
            {
                meshTriangle[a].vertexIndex[0] = cellVertexIndex[classVertexIndex[2]];
                meshTriangle[a].vertexIndex[1] = cellVertexIndex[classVertexIndex[1]];
                meshTriangle[a].vertexIndex[2] = cellVertexIndex[classVertexIndex[0]];
                classVertexIndex += 3;
            }
        }
        else
        {
            for (MFint32 a = 0; a < triangleCount; ++a)
            {
                meshTriangle[a].vertexIndex[0] = cellVertexIndex[classVertexIndex[0]];
                meshTriangle[a].vertexIndex[1] = cellVertexIndex[classVertexIndex[1]];
                meshTriangle[a].vertexIndex[2] = cellVertexIndex[classVertexIndex[2]];
                classVertexIndex += 3;
            }
        }

    }
}

//loads the transition cell face along a constant X value
MFu32 Manifest_Terrain::LoadTransitionCell_X(const Voxel* field, const MFint32& n, const MFint32& m, const MFint32& i, const MFint32& j, const MFint32& k, Voxel* distance, const MFu8& lodScale)
{
    return 0
        | ((std::signbit(static_cast<MFfloat>(distance[0x9] = distance[0] = GetVoxel(field, n, m, i, j, k))) << 0) & 0x01)
        | ((std::signbit(static_cast<MFfloat>(distance[1] = GetVoxel(field, n, m, i, j + (lodScale >> 1), k))) << 1) & 0x02)
        | ((std::signbit(static_cast<MFfloat>(distance[0xA] = distance[2] = GetVoxel(field, n, m, i, j + lodScale, k))) << 2) & 0x04)
        | ((std::signbit(static_cast<MFfloat>(distance[3] = GetVoxel(field, n, m, i, j, k + (lodScale >> 1)))) << 7) & 0x80)
        | ((std::signbit(static_cast<MFfloat>(distance[4] = GetVoxel(field, n, m, i, j + (lodScale >> 1), k + (lodScale >> 1)))) << 8) & 0x100)
        | ((std::signbit(static_cast<MFfloat>(distance[5] = GetVoxel(field, n, m, i, j + lodScale, k + (lodScale >> 1)))) << 3) & 0x08)
        | ((std::signbit(static_cast<MFfloat>(distance[0xB] = distance[6] = GetVoxel(field, n, m, i, j, k + lodScale))) << 6) & 0x40)
        | ((std::signbit(static_cast<MFfloat>(distance[7] = GetVoxel(field, n, m, i, j + (lodScale >> 1), k + lodScale))) << 5) & 0x20)
        | ((std::signbit(static_cast<MFfloat>(distance[0xC] = distance[8] = GetVoxel(field, n, m, i, j + lodScale, k + lodScale))) << 4) & 0x10);
}
//loads the transition cell face along a constant Y value
MFu32 Manifest_Terrain::LoadTransitionCell_Y(const Voxel* field, const MFint32& n, const MFint32& m, const MFint32& i, const MFint32& j, const MFint32& k, Voxel* distance, const MFu8& lodScale)
{
    return 0
        | ((std::signbit(static_cast<MFfloat>(distance[0x9] = distance[0] = GetVoxel(field, n, m, i, j, k))) << 0) & 0x01)
        | ((std::signbit(static_cast<MFfloat>(distance[1] = GetVoxel(field, n, m, i + (lodScale >> 1), j, k))) << 1) & 0x02)
        | ((std::signbit(static_cast<MFfloat>(distance[0xA] = distance[2] = GetVoxel(field, n, m, i + lodScale, j, k))) << 2) & 0x04)
        | ((std::signbit(static_cast<MFfloat>(distance[3] = GetVoxel(field, n, m, i, j, k + (lodScale >> 1)))) << 7) & 0x80)
        | ((std::signbit(static_cast<MFfloat>(distance[4] = GetVoxel(field, n, m, i + (lodScale >> 1), j, k + (lodScale >> 1)))) << 8) & 0x100)
        | ((std::signbit(static_cast<MFfloat>(distance[5] = GetVoxel(field, n, m, i + lodScale, j, k + (lodScale >> 1)))) << 3) & 0x08)
        | ((std::signbit(static_cast<MFfloat>(distance[0xB] = distance[6] = GetVoxel(field, n, m, i, j, k + lodScale))) << 6) & 0x40)
        | ((std::signbit(static_cast<MFfloat>(distance[7] = GetVoxel(field, n, m, i + (lodScale >> 1), j, k + lodScale))) << 5) & 0x20)
        | ((std::signbit(static_cast<MFfloat>(distance[0xC] = distance[8] = GetVoxel(field, n, m, i + lodScale, j, k + lodScale))) << 4) & 0x10);
}
//calcualtes the offset for a given corner sample along the x axis
Integer3D Manifest_Terrain::CalculateOffset_X(const MFu8& corner, const MFu8& lod)
{
    auto generalOffset = TransitionOffsetTable[corner];
    return Integer3D{ 0, ((generalOffset & 0x03)) << (lod - 1), (((generalOffset >> 2) & 0x03)) << (lod - 1) };
}
//calcualtes the offset for a given corner sample along the t axis
Integer3D Manifest_Terrain::CalculateOffset_Y(const MFu8& corner, const MFu8& lod)
{
    auto generalOffset = TransitionOffsetTable[corner];
    return Integer3D{ ((generalOffset & 0x03)) << (lod - 1), 0,(((generalOffset >> 2) & 0x03)) << (lod - 1) };
}