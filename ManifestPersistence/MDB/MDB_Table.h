#pragma once 
#include <string>
#include <vector>

#include <ManifestPersistence/DatabaseTypes.h>
#include <ManifestMemory/Manifest_Allocator.h>

using namespace Manifest_Memory;

namespace Manifest_Persistence
{	

	template<typename MDB_TableType>
	struct DatabaseBuildTable
	{
		//std::map<std::string,PrimaryKey> mappedEntryKeys;
		ScratchPadUMap<ScratchPadString, PrimaryKey> mappedEntryKeys;
		ScratchPadVector<MDB_TableType> entries;
		PrimaryKey nextTableIndex = 0;
	};

	//function placed at the top level in Manifest_Table.h so that the binaries which include the mdb types may have access to the function. binary types do not know about the table and therefore both need to have a high level hand off of this function. 
	template<typename Binary_TableType>
	inline size_t EntrySize(const Binary_TableType& entry)
	{
		return sizeof(Binary_TableType) + entry.header.payloadSize;
	}
}