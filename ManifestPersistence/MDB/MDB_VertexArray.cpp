#include "MDB_VertexArray.h"

using namespace Manifest_Persistence;

ForeignKey Manifest_Persistence::TableEntry(const DDL_Structure& structure, VertexArrayBuildTable& vertexArrayBuildTable)
{	
	MDB_VertexArray& entry = vertexArrayBuildTable.entries.emplace_back();
	entry.vertexArrayID = vertexArrayBuildTable.nextTableIndex++;
	vertexArrayBuildTable.mappedEntryKeys.insert({ structure.name.c_str(),entry.vertexArrayID });
	const DDL_Buffer& bufferData{ HeapData<GEX_VertexArray>(structure).vertexArray.data };
	entry.elements = bufferData.subBufferCount * bufferData.subBufferElements;
	//entry.vertexData = new float[entry.elements];
	entry.vertexData = New<float,ScratchPad<float>>(entry.elements);
	memcpy(entry.vertexData, bufferData.typeHeap, entry.elements * sizeof(float));
	return entry.vertexArrayID;
}