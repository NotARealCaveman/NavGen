#include "GraphicsSystem.h"

using namespace Manifest_Core;

void Manifest_Core::ObservableGraphicsSystem::NotifyNewGraphicsNode(std::vector<Binary_GeometryNode>&& newGeometryNodes, EventSpace& eventSpace)
{
	GraphicsSystemEvent event;
	event.eventToken = UnderlyingType(GraphicsSystemMessageType::NEW_GRAPHICS_NODE);
	event.messages.emplace_back(UnderlyingType(GraphicsSystemMessageType::NEW_GRAPHICS_NODE), std::move(newGeometryNodes));
	eventSpace.NotifyRegisteredObservers(std::move(event));
}