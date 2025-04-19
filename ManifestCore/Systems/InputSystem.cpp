#include "InputSystem.h"

using namespace Manifest_Core;

//action 0 = press, 1 = repeat, 2 = release
void Manifest_Core::ObservableInputSystem::NotifyKeyCallback(const InputCode& keyCode, const MFint32& action, EventSpace& eventSpace)
{	
	KeyboardMessage keyboardMessage;
	keyboardMessage.keyCode = keyCode;
	keyboardMessage.messageSource = KeyboardMessage::CALLBACK_MESSAGE;
	switch (action)
	{
		case ManifsetActionCodes::PRESS:
			keyboardMessage.action = InputSystemMessageType::KEY_PRESS;
			break;
		case ManifsetActionCodes::REPEAT:
			keyboardMessage.action = InputSystemMessageType::KEY_REPEAT;
			break;
		case ManifsetActionCodes::RELEASE:
			keyboardMessage.action = InputSystemMessageType::KEY_RELEASE;
			break;
		default:
			assert(("invliad input system message type", false));
	}
	InputSystemEvent event;
	event.eventToken = UnderlyingType(keyboardMessage.action);
	event.messages.emplace_back(event.eventToken, keyboardMessage);
	eventSpace.NotifyRegisteredObservers(std::move(event));
}

void Manifest_Core::ObservableInputSystem::NotifyKeyPolling(const InputCode& keyCode, const MFint32& action, EventSpace& eventSpace)
{
	KeyboardMessage keyboardMessage;
	keyboardMessage.keyCode = keyCode;
	keyboardMessage.messageSource = KeyboardMessage::POLLED_MESSAGE;
	switch (action)
	{
	case ManifsetActionCodes::PRESS:
		keyboardMessage.action = InputSystemMessageType::KEY_PRESS;
		break;
	case ManifsetActionCodes::REPEAT:
		keyboardMessage.action = InputSystemMessageType::KEY_REPEAT;
		break;
	case ManifsetActionCodes::RELEASE:
		keyboardMessage.action = InputSystemMessageType::KEY_RELEASE;
		break;
	default:
		assert(("invliad input system message type", false));
	}
	InputSystemEvent event;
	event.eventToken = UnderlyingType(keyboardMessage.action);
	event.messages.emplace_back(event.eventToken, keyboardMessage);
	eventSpace.NotifyRegisteredObservers(std::move(event));
}

void Manifest_Core::ObservableInputSystem::NotifyMouseCallback(const InputCode& button, const MFint32& action, const MFfloat xScreenSpacePosition, const MFfloat yScreenSpacePosition, EventSpace& eventSpace)
{
	MouseMessage mouseMessage;
	mouseMessage.buttonNumber = button;	
	mouseMessage.xScreenSpacePosition = xScreenSpacePosition;
	mouseMessage.yScreenSpacePosition = yScreenSpacePosition;
	switch (action)
	{
	case ManifsetActionCodes::PRESS:
		mouseMessage.action = InputSystemMessageType::MOUSE_PRESS;
		break;
	case ManifsetActionCodes::RELEASE:
		mouseMessage.action = InputSystemMessageType::MOUSE_RELEASE;
		break;
	default:
		assert(("invliad input system message type", false));
	}
	InputSystemEvent event;
	event.eventToken = UnderlyingType(mouseMessage.action);
	event.messages.emplace_back(event.eventToken, mouseMessage);
	eventSpace.NotifyRegisteredObservers(std::move(event));
}
