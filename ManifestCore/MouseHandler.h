#pragma once
#include <atomic>
#include <array>	
#include <utility>

#include <ManifestCore/Systems/InputSystem.h>
#include <ManifestMemory/Utility/FunctionBinder.h>
#include <ManifestMath/Vector2.h>

#include <ManifestMath/Point3.h>

#include <ManifestUtility/DebugLogger.h>
#include <ManifestUtility/TypeAssist.h>

using namespace Manifest_Utility;
using namespace Manifest_Communication;
using Manifest_Math::MFvec2, Manifest_Math::MFpoint3, Manifest_Math::MFvec3;


namespace Manifest_Core
{
	constexpr MFu8 MAX_MOUSE_BUTTONS{ 5 };
	using MouseButtonFunction = FunctionBinder;
	using MouseButtonFunctionSet = std::array<MouseButtonFunction, 2>;	

	struct Button
	{		
		MFu8 state;//released or pressed
		MFvec2 lastPosition;//position the most recent message was recv'd at
		MouseButtonFunctionSet functions;//press/release button functions
	};

	struct MouseState
	{
		MFvec2 mouseScreenSpacePosition;
		MFpoint3 mouseWorldSpacePosition;
		MFvec3 mouseWorldSpaceDirection;		
	};

	struct MouseStateSynchronizer
	{
		static std::atomic<std::shared_ptr<MouseState>> newMouseState;
	};

	class MouseHandler
	{
	public:
		MouseHandler();				
		void RegisterMouseButtonFunction(const InputSystemMessageType& mouseEventType, const InputCode buttonNumber, MouseButtonFunction&& buttonFunction);
		void ObserveInputSystemMessages();		
		const Button& GetButton(const InputCode buttonNumber) const;
		const MFvec2 GetMouseScreenPosition() const;
		const MFpoint3 GetMouseWorldPosition() const;
		const MFvec3 GetMouseWorldDirection() const;
		const MouseState& GetMouseState() const;
				
		InputSystemObserver inputSystemObserver;		
	private:		
		void ProcessMousePressEvent(const MouseMessage& mouseMessage);
		void ProcessMouseReleaseEvent(const MouseMessage& mouseMessage);		
		void ProcessMouseEvent(const MouseMessage& mouseMessage, const MFu8 buttonState);
		static constexpr InputSystemObservationToken inputSystemObservationToken
		{
			UnderlyingType(InputSystemMessageType::MOUSE_PRESS | InputSystemMessageType::MOUSE_RELEASE)
		};						
		static constexpr MFu8 PRESSED{ 0 };
		static constexpr MFu8 RELEASED{ 1 };
		std::array<Button, MAX_MOUSE_BUTTONS> buttons;			
		MouseState mouseState;
	};
}