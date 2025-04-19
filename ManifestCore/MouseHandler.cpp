#include "MouseHandler.h"

using namespace Manifest_Core;

std::atomic<std::shared_ptr<MouseState>> MouseStateSynchronizer::newMouseState{};

MouseHandler::MouseHandler() : inputSystemObserver{ inputSystemObservationToken }
{
	std::ranges::for_each(buttons, [&](Button& button)
		{
			std::ranges::generate(button.functions, []()->MouseButtonFunction
				{
					return MouseButtonFunction{ []() {} };
				});
		});
}

void MouseHandler::RegisterMouseButtonFunction(const InputSystemMessageType& mouseEventType, const InputCode buttonNumber, MouseButtonFunction&& buttonFunction)
{	
	if (buttonNumber > MAX_MOUSE_BUTTONS)
		return;

	Button& button{ buttons[buttonNumber] };

	MFu8 invokableFunctionState{ std::numeric_limits<MFu8>::max()};
	switch (mouseEventType)
	{
		case InputSystemMessageType::MOUSE_PRESS:
			invokableFunctionState = PRESSED;
			break;
		case InputSystemMessageType::MOUSE_RELEASE:
			invokableFunctionState = RELEASED;
			break;
		default:	
			assert("Invalid event type supplied:", invokableFunctionState == std::numeric_limits<MFu8>::max());
	}
		
	button.functions[invokableFunctionState] = std::move(buttonFunction);	
}

void MouseHandler::ObserveInputSystemMessages()
{
	//check for new mouse state
	std::shared_ptr<MouseState> newMouseState{ MouseStateSynchronizer::newMouseState.exchange(nullptr,std::memory_order_acq_rel) };
	if (newMouseState)
		mouseState = *newMouseState;

	if (!inputSystemObserver.HasPendingMessages())
		return;

	inputSystemObserver.ProcessEvents([&](std::vector<Message>& messages)
		{
			std::ranges::for_each(messages, [&](Message& message)
				{
					const MouseMessage mouseMessage{ message.GetMessageContent<const MouseMessage>() };
					switch (message.messageToken)
					{		
						//USED FOR INITIATING MOUSE BUTTON EVENTS
					case UnderlyingType(InputSystemMessageType::MOUSE_RELEASE):
						ForwardFunction(&MouseHandler::ProcessMouseReleaseEvent,*this,mouseMessage);
						break;
						//USED FOR PROMPTING MOUSE SELECTION 
					case UnderlyingType(InputSystemMessageType::MOUSE_PRESS):
						ForwardFunction(&MouseHandler::ProcessMousePressEvent, *this, mouseMessage);
						break;

					default:
						break;
					}
				});
		});
}

void MouseHandler::ProcessMousePressEvent(const MouseMessage& mouseMessage) 
{
	return ProcessMouseEvent(mouseMessage, PRESSED);
}

void MouseHandler::ProcessMouseReleaseEvent(const MouseMessage& mouseMessage) 
{
	return ProcessMouseEvent(mouseMessage, RELEASED);
}

void MouseHandler::ProcessMouseEvent(const MouseMessage& mouseMessage, const MFu8 buttonState) 
{
	const InputCode buttonNumber{ mouseMessage.buttonNumber};
	assert(buttonNumber < MAX_MOUSE_BUTTONS);		

	Button& button{ buttons[buttonNumber] };
	button.lastPosition.x = mouseMessage.xScreenSpacePosition;
	button.lastPosition.y = mouseMessage.yScreenSpacePosition;
	button.state = buttonState;	
	const MouseButtonFunction& mouseReleaseFunction{ button.functions[button.state]};	
	ForwardFunction(mouseReleaseFunction);
}

const Button& MouseHandler::GetButton(const InputCode buttonNumber) const
{
	assert(buttonNumber < MAX_MOUSE_BUTTONS);
	return buttons[buttonNumber];
}

const MFvec2 MouseHandler::GetMouseScreenPosition() const
{
	return mouseState.mouseScreenSpacePosition;
}

const MFpoint3 MouseHandler::GetMouseWorldPosition() const
{
	return mouseState.mouseWorldSpacePosition;
}

const MFvec3 MouseHandler::GetMouseWorldDirection() const
{
	return mouseState.mouseWorldSpaceDirection;
}

const MouseState& MouseHandler::GetMouseState() const
{
	return mouseState;
}