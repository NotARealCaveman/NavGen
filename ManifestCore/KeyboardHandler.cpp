#include "KeyboardHandler.h"


using namespace Manifest_Core;

KeyboardHandler::KeyboardHandler()
	: inputSystemObserver{ inputSystemObservationToken }
{

} 

void KeyboardHandler::Shutdown()
{
	mappedPollingFunctions.clear();
	mappedCallbackFunctions.clear();
}

void KeyboardHandler::RegisterKeyboardFunction(const InputSystemMessageType& keyboardEventType, const InputCode keyCode, KeyboardFunction&& keyboardFunction, MFbool callbackFunction)
{
	MFu8 eventType{ GetKeyboardAction(keyboardEventType) };

	std::unordered_map<InputCode, KeyboardFunctionSet>& mappedKeyboardFunctions{ callbackFunction ? mappedCallbackFunctions : mappedPollingFunctions };

	if (!mappedKeyboardFunctions.contains(keyCode))
	{
		KeyboardFunctionSet keyboardFunctionSet{ DEFAULT_KEYBOARD_FUNCTION,DEFAULT_KEYBOARD_FUNCTION,DEFAULT_KEYBOARD_FUNCTION };
		keyboardFunctionSet[eventType] = std::move(keyboardFunction);
		mappedKeyboardFunctions.insert(std::make_pair(keyCode, std::move(keyboardFunctionSet)));
	}
	else
		mappedKeyboardFunctions[keyCode][eventType] = std::move(keyboardFunction);
}

void KeyboardHandler::ObserveInputSystemMessages()
{
	if (!inputSystemObserver.HasPendingMessages())
		return;

	inputSystemObserver.ProcessEvents([&](std::vector<Message>& messages)
		{			
			std::ranges::for_each(messages, [&](Message& message)
				{					
					KeyboardMessage keyboardMessage = message.GetMessageContent<KeyboardMessage>();
					const InputCode& keyCode{ keyboardMessage.keyCode };
					std::unordered_map<InputCode, KeyboardFunctionSet>* mappedKeyboardFunctions{ nullptr };
					switch (keyboardMessage.messageSource)
					{
						case KeyboardMessage::CALLBACK_MESSAGE:
							mappedKeyboardFunctions = &mappedCallbackFunctions;
							break;
						case KeyboardMessage::POLLED_MESSAGE:									
							mappedKeyboardFunctions = &mappedPollingFunctions;
							break;						
					}
					auto functionSetIterator{ mappedKeyboardFunctions->find(keyCode) };
					if (functionSetIterator == mappedKeyboardFunctions->end())
						return;
					MFu8 eventType{ GetKeyboardAction(keyboardMessage.action) };	
					const KeyboardFunction& keyboardFunction{(*mappedKeyboardFunctions)[keyCode][eventType]};
					ForwardFunction(keyboardFunction);
				});
		});
}

MFu8 Manifest_Core::GetKeyboardAction(const InputSystemMessageType& keyboardEventType)
{
	switch (keyboardEventType)
	{
		case InputSystemMessageType::KEY_PRESS:
			return ManifsetActionCodes::PRESS;
		case InputSystemMessageType::KEY_RELEASE:
			return ManifsetActionCodes::RELEASE;
		case InputSystemMessageType::KEY_REPEAT:
			return ManifsetActionCodes::REPEAT;
		case InputSystemMessageType::MOUSE_PRESS: case InputSystemMessageType::MOUSE_RELEASE:
			assert(("invliad input system messagetype", false));
	}
}