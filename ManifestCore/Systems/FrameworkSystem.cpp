#include "FrameworkSystem.h"

using namespace Manifest_Core;

void ObservableFrameworkSystem::AddCollisionVolume(Event& event, Collider* collider)
{
	event.eventToken |= UnderlyingType(MessageTypes::ADD_COLLIDER);
	event.messages.emplace_back(Message {UnderlyingType(MessageTypes::ADD_COLLIDER), std::move(collider)});
}

void ObservableFrameworkSystem::AttachGraphicsResource(Event& event, GraphicsAttachment graphicsAttachment)
{
	event.eventToken |= UnderlyingType(MessageTypes::ATTACH_GRAPHICS);
	event.messages.emplace_back(Message {UnderlyingType(MessageTypes::ATTACH_GRAPHICS), graphicsAttachment });
}