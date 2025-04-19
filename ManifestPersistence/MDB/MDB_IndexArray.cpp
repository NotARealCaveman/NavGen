#include "MDB_IndexArray.h"

using namespace Manifest_Persistence;

ForeignKey Manifest_Persistence::TableEntry(const DDL_Structure& structure, IndexBuildTable& indexBuildTable)
{	
	MDB_IndexArray& entry = indexBuildTable.entries.emplace_back();
	entry.indexArrayID = indexBuildTable.nextTableIndex++;
	indexBuildTable.mappedEntryKeys.insert({ structure.name.c_str(),entry.indexArrayID });
	const DDL_Buffer& bufferData{ HeapData<GEX_VertexArray>(structure).vertexArray.data };
	entry.elements = bufferData.subBufferCount * bufferData.subBufferElements;
	//entry.indexData = new uint32_t[entry.elements];
	entry.indexData = New<MFu32,ScratchPad<MFu32>>(entry.elements);
	memcpy(entry.indexData, bufferData.typeHeap, entry.elements * sizeof(uint32_t));
	return entry.indexArrayID;
}