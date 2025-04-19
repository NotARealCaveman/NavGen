#pragma once
#include <ManifestCommunication/EventSpace.h>
#include <ManifestPersistence/MDB/Binary/Binary_GeometryNode.h>

using namespace Manifest_Communication;
using namespace Manifest_Persistence;

namespace Manifest_Core
{
	struct ObservableGraphicsSystem
	{
		enum class MessageTypes : MFu64
		{
			NEW_GRAPHICS_NODE = pow2(0)
		};
		static void NotifyNewGraphicsNode(std::vector<Binary_GeometryNode>&& newGeometryNodes, EventSpace& eventSpace);
	};

	using GraphicsSystem = ObservableGraphicsSystem;
	using GraphicsSystemMessageType = GraphicsSystem::MessageTypes;
	using GraphicsSystemObservationToken = ObservationToken;
	using GraphicsSystemEvent = Event;
	using GraphicsSystemObserver = Observer;
	using GraphicsSystemEventSpace = EventSpace;
}