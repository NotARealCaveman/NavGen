#pragma once

#include <ManifestPersistence/Runtime_Database.h>
using namespace Manifest_Persistence;

namespace Manifest_Core
{
	class EngineModule
	{
		virtual void Pull() = 0;
		virtual void Push() = 0;
	};
}
