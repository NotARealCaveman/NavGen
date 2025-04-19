#include "MarchingCubes.h"

using namespace Manifest_Terrain;

void Manifest_Terrain::ExtractPrimaryTerrainData(const Voxel* field, const Integer3D& minCorner, const MFint32 n, const MFint32 m, const MFu8 extractionLOD, const MFu8 worldLOD, PrimaryTerrain& primaryTerrain)
{    
    const MFint32 NUMBER_CELLS = BASE_CELLS_PER_BLOCK << worldLOD - extractionLOD;
    
    CellStorage* deckStorage[2];
    //allocate storage for two decks of history
    CellStorage* precedingCellStorage = new CellStorage[NUMBER_CELLS * NUMBER_CELLS * 2];
    MFu16 deltaMask = 0;
    auto target = std::min(NUMBER_CELLS, ceilingLevel);
    for (MFint32 k = 0; k < NUMBER_CELLS; ++k)
    {
        //ping-pong between history decks
        deckStorage[0] = &precedingCellStorage[NUMBER_CELLS * NUMBER_CELLS * (k & 1)];
        for (MFint32 j = 0; j < NUMBER_CELLS; ++j)
        {
            for (MFint32 i = 0; i < NUMBER_CELLS; ++i)
            {
                ProcessCell(field, minCorner,n, m, NUMBER_CELLS, NUMBER_CELLS, i, j, k, deckStorage, deltaMask, extractionLOD, primaryTerrain);
                deltaMask |= 1;//allow reuse in x direction  
            }
            deltaMask = (deltaMask | 2) & 6;//allow reuse in y direction
        }
        deckStorage[1] = deckStorage[0];//current deck becomes preceding deck
        deltaMask = 4;//allow reuse only in z direction
    }
    delete[] precedingCellStorage;
}

void Manifest_Terrain::ProcessCell(const Voxel* field, const Integer3D& minCorner, const MFint32 nField, const MFint32 mField, const MFint32 nVoxels, const MFint32 mVoxels, const MFint32 i, const MFint32 j, const MFint32 k, CellStorage* const(&deckStorage)[2], const MFu32 deltaMask,const MFu8 lod, PrimaryTerrain& primaryTerrain)
{
    MFu8 faceMask = 0;
    if (lod)
    {//no k/z transitions are allowed to occur - design reasoning        
        faceMask |= (i == 0x00) << 0 | (i == nVoxels - 1) << 2 | (j == 0x00)
            << 1 | (j == mVoxels - 1) << 3;
    }
    Voxel distance[8];
    //get storage for current cell aand set vertex indices at corners to invalid values so those not generated here won't get reused    
    CellStorage* cellStorage = &deckStorage[0][j * nVoxels + i];
    for (MFint32 a = 0; a < 7; ++a)
        cellStorage->corner[a] = 0xFFFF;
    //4.2 Geomip terrain meshes - voxels are sampled at double rate
    //for each lod level
    MFu8 lodScale = 1 << lod;
    const MFint32 iOffset = minCorner.i + (i << lod);
    const MFint32 jOffset = minCorner.j + (j << lod);
    const MFint32 kOffset = minCorner.k + (k << lod);    
    //load the cell to populate the dsistance array and get the casae index
    MFu32 caseIndex = LoadCell(field, nField, mField, iOffset, jOffset, kOffset, distance, lodScale);
    //look up the equivalence class index and use it to look up the geometric data for this cell. no geometry if case is 0 or 255
    MFu32 equivalenceClass = regularCellClass[caseIndex];
    const RegularCellData* classData = &regularCellData[equivalenceClass];
    MFu32 geometryCounts = classData->geometryCounts;
    if (geometryCounts)
    {
        MFu32 cellVertexIndex[12];
        MFint32 vertexCount = geometryCounts >> 4;
        MFint32 triangleCount = geometryCounts & 0x0F;
        //look up vertex codes using original case index
        const MFu16* vertexCode = vertexCodeTable[caseIndex];
        //duplicate middle bit of delta mask to construct 4-bit mask used for edges
        MFu16 edgeDeltaMask = ((deltaMask << 1) & 0x0C) | (deltaMask & 0x03);
        for (MFint32 a = 0; a < vertexCount; ++a)
        {
            MFu32 vertexIndex;
            MFu8 corner[2];
            Integer3D position[2];

            //extract corner numbers from low 6 bits of vertex code
            MFu16 vCode = vertexCode[a];
            corner[0] = vCode & 0x07;
            corner[1] = (vCode >> 3) & 0x07;
            //3.4 & 3.5 calculate interpolation parameter
            Voxel d0 = distance[corner[0]];
            Voxel d1 = distance[corner[1]];
            MFint32 t = (d1 << WORLD_INTERPOLATION) / (d1 - d0);
            //construct integer coordinates of edge's endpoints  
            //4.2 Geomip maps block size is doubled thus cells are doubled per lod
            position[0].i = (iOffset)+((corner[0] & 1) << lod);
            position[0].j = (jOffset)+(((corner[0] >> 1) & 1) << lod);
            position[0].k = (kOffset)+(((corner[0] >> 2) & 1) << lod);
            position[1].i = (iOffset)+((corner[1] & 1) << lod);
            position[1].j = (jOffset)+(((corner[1] >> 1) & 1) << lod);
            position[1].k = (kOffset)+(((corner[1] >> 2) & 1) << lod);            
            if (t)
            {
                //4.2.1 Surface Shifting
                if (lod)
                {
                    //determine midpoint sample                
                    SurfaceShift(lod, position[0], position[1], field, nField, mField, corner, d0, d1);
                    t = (d1 << 8) / (d1 - d0);
                }
                //vertex falls in the interior of an edge extract edge index and delta code from vertex code
                MFu16 edgeIndex = (vCode >> 8) & 0x0F;
                MFu16 deltaCode = (vCode >> 12) & edgeDeltaMask;
                //reuse vertex from edge in preceeding cell
                if (deltaCode)
                    vertexIndex = ReuseEdgeVertex(nVoxels, i, j, deckStorage, edgeIndex, deltaCode);
                //generate a new vertex 
                else
                {
                    vertexIndex = primaryTerrain.extractionData.nVertices++;
                    Integer3D* vertex = &primaryTerrain.extractionData.vertices[vertexIndex];
                    *vertex = ((position[0]) * t + (position[1]) * (0x0100 - t));
                    //store vertex index for potential reuse
                    if (edgeIndex >= 3)
                        cellStorage->edge[edgeIndex - 3] = vertexIndex;
                }
                cellVertexIndex[a] = vertexIndex;                
            }
            else
            {
                //vertex falls exactly at the first corner of the cell if t==0 and the second corner if t==0x0100
                MFu8 c = t == 0;
                MFu8 cornerIndex = corner[c];
                //corner vertex in preceding cellmay not have been generate so we get address and look for valid index
                MFu32* indexAddress = nullptr;
                MFu16 deltaCode = (cornerIndex ^ 7) & deltaMask;
                //reuse vertex from corner in preceding cell
                if (deltaCode)
                    indexAddress = ReuseCornerVertex(nVoxels, i, j, deckStorage, cornerIndex, deltaCode);
                //vertex will be stored for potential use later
                else if (cornerIndex)
                    indexAddress = &cellStorage->corner[cornerIndex - 1];
                vertexIndex = indexAddress ? *indexAddress : 0xFFFF;
                //vertex was not previosuly generated
                if (vertexIndex == 0xFFFF)
                {
                    vertexIndex = primaryTerrain.extractionData.nVertices++;
                    if (indexAddress)
                        *indexAddress = vertexIndex;
                    //shift corner position to add 8 bits of fraction
                    primaryTerrain.extractionData.vertices[vertexIndex] = (position[c] << 8);
                }
                cellVertexIndex[a] = vertexIndex;                
            }            
            //first check if this vertex is on a shared corner
            auto transitionCombination = TransitionCombinationTable[faceMask];            
            if (transitionCombination.transitionCount)//should only ever get 1 shared corner at most here
                primaryTerrain.sharedFaceVertexIndexMap[vertexIndex] = *transitionCombination.cornerCombinations;
            //then check if it's along any part of a face
            else if (faceMask)
                for (auto face = 0; face < NUMBER_TRANSITION_FACES; ++face)
                    if (faceMask & (1 << face))
                    {
                        primaryTerrain.faceVertexIndexMap[vertexIndex] = face;
                        break;
                    }
        }
        //generate triangles for this cell using table data     
        const MFu8* classVertexIndex = classData->vertexIndex;
        Triangle* meshTriangle = &primaryTerrain.extractionData.triangles[primaryTerrain.extractionData.nTriangles];
        primaryTerrain.extractionData.nTriangles += triangleCount;
        for (MFint32 a = 0; a < triangleCount; ++a)
        {
            meshTriangle[a].vertexIndex[0] = cellVertexIndex[classVertexIndex[0]];
            meshTriangle[a].vertexIndex[1] = cellVertexIndex[classVertexIndex[1]];
            meshTriangle[a].vertexIndex[2] = cellVertexIndex[classVertexIndex[2]];
            classVertexIndex += 3;
        }
    }
}

MFu32 Manifest_Terrain::LoadCell(const Voxel* field, const MFint32& n, const MFint32& m, const MFint32& i, const MFint32& j, const MFint32& k, Voxel* distance, const MFu8& lodscale)
{
    return 0
        | ((std::signbit(static_cast<MFfloat>(distance[0] = GetVoxel(field, n, m, i, j, k))) << 0) & 0x01)
        | ((std::signbit(static_cast<MFfloat>(distance[1] = GetVoxel(field, n, m, i + lodscale, j, k))) << 1) & 0x02)
        | ((std::signbit(static_cast<MFfloat>(distance[2] = GetVoxel(field, n, m, i, j + lodscale, k))) << 2) & 0x04)
        | ((std::signbit(static_cast<MFfloat>(distance[3] = GetVoxel(field, n, m, i + lodscale, j + lodscale, k))) << 3) & 0x08)
        | ((std::signbit(static_cast<MFfloat>(distance[4] = GetVoxel(field, n, m, i, j, k + lodscale))) << 4) & 0x10)
        | ((std::signbit(static_cast<MFfloat>(distance[5] = GetVoxel(field, n, m, i + lodscale, j, k + lodscale))) << 5) & 0x20)
        | ((std::signbit(static_cast<MFfloat>(distance[6] = GetVoxel(field, n, m, i, j + lodscale, k + lodscale))) << 6) & 0x40)
        | ((std::signbit(static_cast<MFfloat>(distance[7] = GetVoxel(field, n, m, i + lodscale, j + lodscale, k + lodscale))) << 7) & 0x80);
}

void Manifest_Terrain::SurfaceShift(const MFu8& currentLoD, Integer3D& minSample, Integer3D& maxSample, const Voxel* field, const MFu32& nField, const MFu32 mField, const MFu8(&corner)[2], Voxel& d0, Voxel& d1)
{
    if (!currentLoD)
        return;
    auto midSample = (minSample + maxSample) >> 1;
    auto sample = GetVoxel(field, nField, mField, midSample.i, midSample.j, midSample.k);
    //determine sub edge from current edge voxel values
    if (d0 < 0)//lower corner is currently inside the terrain
    {
        if (sample < 0)//sample is also inside the terrain - choose upper subedge
        {
            minSample = midSample;//update subedge min corner
            d0 = sample;//update min corner voxel sample
        }//sample is outside the terrain - choose lower subedge
        else
        {
            maxSample = midSample;
            d1 = sample;
        }
    }//upper corner is currently inside the terrain
    else
    {
        if (sample < 0)//sample is also inside the terrain - choose lower subedge
        {
            maxSample = midSample;
            d1 = sample;
        }//sample is outside the terrain - choose upper subedge
        else
        {
            minSample = midSample;
            d0 = sample;
        }
    }
    SurfaceShift(currentLoD - 1, minSample, maxSample, field, nField, mField, corner, d0, d1);
}

Integer3D Manifest_Terrain::ValidationNormal(const Integer3D& a, const Integer3D& b, const Integer3D& c, const Integer3D& minCorner)
{
    //remove scaled voxel space offset
    const Integer3D i{ a - (minCorner << 8) };
    const Integer3D j{ b - (minCorner << 8) };
    const Integer3D k{ c - (minCorner << 8) };
    //get surface normal of the triangle
    const Integer3D result{ Cross((i - j), (i - k))};    

    return result;
}