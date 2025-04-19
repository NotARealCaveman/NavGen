#pragma once
#include <unordered_map>
#include <array>

#include <ManifestMemory/Utility/FunctionBinder.h>
#include <ManifestCore/Systems/InputSystem.h>
#include <ManifestUtility/DebugLogger.h>

using namespace Manifest_Utility;
using namespace Manifest_Communication;

namespace Manifest_Core
{		
	using KeyboardFunction = FunctionBinder;
	using KeyboardFunctionSet = std::array<KeyboardFunction, 3>;
	inline void DEFAULT_KEYBOARD_FUNCTION() {}; 
	 
	struct PolledKey
	{
		InputCode keyCode;
		MFu8 action;
	};

	class KeyboardHandler
	{
	public:
		KeyboardHandler();	
		void Shutdown();
		//callback  - invokes function when key is triggered		
		//polled - invokes function based on state of key
		void RegisterKeyboardFunction(const InputSystemMessageType& keyboardEventType, const InputCode keyCode, KeyboardFunction&& keyboardFunction, MFbool callbackFunction);
		void ObserveInputSystemMessages();		


		InputSystemObserver inputSystemObserver;
	private:		
		static constexpr InputSystemObservationToken inputSystemObservationToken
		{
			UnderlyingType(InputSystemMessageType::KEY_PRESS | InputSystemMessageType::KEY_RELEASE | InputSystemMessageType::KEY_REPEAT)
		};	
		//MAPS TO THE KEYBAORD FUNCTION INDEX NOT THE ACTION VALUES
		static constexpr MFu8 KEY_PRESS_ACTION{ 0 };
		static constexpr MFu8 KEY_REPEAT_ACTION{ 1 };
		static constexpr MFu8 KEY_RELEASE_ACTION{ 2 };
		std::unordered_map<InputCode, KeyboardFunctionSet> mappedCallbackFunctions;
		std::unordered_map<InputCode, KeyboardFunctionSet> mappedPollingFunctions;
	};
	MFu8 GetKeyboardAction(const InputSystemMessageType& keyboardEventType);
}