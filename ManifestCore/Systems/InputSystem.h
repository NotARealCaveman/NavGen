#pragma once
#include <ManifestCommunication/EventSpace.h>
#include <ManifestCore/InputSupport.h>

using namespace Manifest_Communication;

namespace Manifest_Core
{
	struct ObservableInputSystem
	{
	private:
		static constexpr MFu8 KEY_PRESSED{ 0 };
		static constexpr MFu8 KEY_REPEATING{ KEY_PRESSED + 1 };
		static constexpr MFu8 KEY_RELEASED{ KEY_REPEATING + 1 };
		static constexpr MFu8 MOUSE_PRESSED{ KEY_RELEASED + 1 };
		static constexpr MFu8 MOUSE_RELEASED{ MOUSE_PRESSED + 1 };
	public:
		enum class MessageTypes : MFu8
		{
			KEY_PRESS = pow2(KEY_PRESSED),
			KEY_REPEAT = pow2(KEY_REPEATING),
			KEY_RELEASE = pow2(KEY_RELEASED),
			MOUSE_PRESS = pow2(MOUSE_PRESSED),
			MOUSE_RELEASE = pow2(MOUSE_RELEASED),
		};
		static void NotifyKeyCallback(const InputCode& keyCode, const MFint32& action, EventSpace& eventSpace);
		static void NotifyKeyPolling(const InputCode& keyCode, const MFint32& action,EventSpace& eventSpace);
		static void NotifyMouseCallback(const InputCode& button, const MFint32& action, const MFfloat xScreenSpacePosition, const MFfloat yScreenSpacePosition, EventSpace& eventSpace);
	};

	using InputSystem = ObservableInputSystem;
	using InputSystemMessageType = InputSystem::MessageTypes;
	using InputSystemObservationToken = ObservationToken;
	using InputSystemEvent = Event;
	using InputSystemObserver = Observer;
	using InputSystemEventSpace = EventSpace;	

	struct KeyboardMessage
	{
		static constexpr MFu8 POLLED_MESSAGE{ 0 };
		static constexpr MFu8 CALLBACK_MESSAGE{ 1 };

		InputSystemMessageType action;//press,repeat,release
		InputCode keyCode;
		MFu8 messageSource;
	};

	struct MouseMessage
	{		
		InputSystemMessageType action;//press,release		
		MFfloat xScreenSpacePosition;
		MFfloat yScreenSpacePosition;
		InputCode buttonNumber;
	};
}