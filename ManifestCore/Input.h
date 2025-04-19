#pragma once
#include <unordered_map>
#include <functional>	

#include <GLFW/glfw3.h>

#include <ManifestUtility/Typenames.h>

using namespace Manifest_Utility;

namespace Manifest_Core
{			
	using InputCode = MFint32;

	//MAPPINGS FROM GLFW - DONE TO MATCH UP EXPECTED KEYBOARD INPUT

	namespace ManifsetActionCodes
	{
		/*key actions*/
		static constexpr InputCode RELEASE{ GLFW_RELEASE };
		static constexpr InputCode PRESS{ GLFW_PRESS };
		static constexpr InputCode REPEAT{ GLFW_REPEAT };
	}

	namespace ManifestKeyCodes
	{			
		/* Printable keys */
		static constexpr InputCode SPACE{ GLFW_KEY_SPACE };
		static constexpr InputCode APOSTROPHE{ GLFW_KEY_APOSTROPHE };/* ' */
		static constexpr InputCode COMMA{ GLFW_KEY_COMMA };/* , */
		static constexpr InputCode MINUS{ GLFW_KEY_MINUS };/* - */
		static constexpr InputCode PERIOD{ GLFW_KEY_PERIOD };/* . */
		static constexpr InputCode SLASH{ GLFW_KEY_SLASH };/* / */
		static constexpr InputCode ZERO{ GLFW_KEY_0 };
		static constexpr InputCode ONE{ GLFW_KEY_1 };
		static constexpr InputCode TWO{ GLFW_KEY_2 };
		static constexpr InputCode THREE{ GLFW_KEY_3 };
		static constexpr InputCode FOUR{ GLFW_KEY_4 };
		static constexpr InputCode FIVE{ GLFW_KEY_5 };
		static constexpr InputCode SIX{ GLFW_KEY_6 };
		static constexpr InputCode SEVEN{ GLFW_KEY_7 };
		static constexpr InputCode EIGHT{ GLFW_KEY_8 };
		static constexpr InputCode NINE{ GLFW_KEY_9 };
		static constexpr InputCode SEMICOLON{ GLFW_KEY_SEMICOLON };/* ; */
		static constexpr InputCode EQUAL{ GLFW_KEY_EQUAL };/* = */
		static constexpr InputCode A{ GLFW_KEY_A };
		static constexpr InputCode B{ GLFW_KEY_B };
		static constexpr InputCode C{ GLFW_KEY_C };
		static constexpr InputCode D{ GLFW_KEY_D };
		static constexpr InputCode E{ GLFW_KEY_E };
		static constexpr InputCode F{ GLFW_KEY_F };
		static constexpr InputCode G{ GLFW_KEY_G };
		static constexpr InputCode H{ GLFW_KEY_H };
		static constexpr InputCode I{ GLFW_KEY_I };
		static constexpr InputCode J{ GLFW_KEY_J };
		static constexpr InputCode K{ GLFW_KEY_K };
		static constexpr InputCode L{ GLFW_KEY_L };
		static constexpr InputCode M{ GLFW_KEY_M };
		static constexpr InputCode N{ GLFW_KEY_N };
		static constexpr InputCode O{ GLFW_KEY_O };
		static constexpr InputCode P{ GLFW_KEY_P };
		static constexpr InputCode Q{ GLFW_KEY_Q };
		static constexpr InputCode R{ GLFW_KEY_R };
		static constexpr InputCode S{ GLFW_KEY_S };
		static constexpr InputCode T{ GLFW_KEY_T };
		static constexpr InputCode U{ GLFW_KEY_U };
		static constexpr InputCode V{ GLFW_KEY_V };
		static constexpr InputCode W{ GLFW_KEY_W };
		static constexpr InputCode X{ GLFW_KEY_X };
		static constexpr InputCode Y{ GLFW_KEY_Y };
		static constexpr InputCode Z{ GLFW_KEY_Z };
		static constexpr InputCode LEFT_BRACKET{ GLFW_KEY_LEFT_BRACKET };  /* [ */
		static constexpr InputCode BACKSLASH{ GLFW_KEY_BACKSLASH }; /* \ */
		static constexpr InputCode RIGHT_BRACKET{ GLFW_KEY_RIGHT_BRACKET };  /* ] */
		/* Function keys */
		static constexpr InputCode ESCAPE{ GLFW_KEY_ESCAPE };
		static constexpr InputCode ENTER{ GLFW_KEY_ENTER };
		static constexpr InputCode TAB{ GLFW_KEY_TAB };
		static constexpr InputCode BACKSPACE{ GLFW_KEY_BACKSPACE };
		static constexpr InputCode INSERT{ GLFW_KEY_INSERT };
		static constexpr InputCode DELETE{ GLFW_KEY_DELETE };
		static constexpr InputCode RIGHT{ GLFW_KEY_RIGHT };
		static constexpr InputCode LEFT{ GLFW_KEY_LEFT };
		static constexpr InputCode DOWN{ GLFW_KEY_DOWN };
		static constexpr InputCode UP{ GLFW_KEY_UP };
		static constexpr InputCode PAGE_UP{ GLFW_KEY_PAGE_UP };
		static constexpr InputCode PAGE_DOWN{ GLFW_KEY_PAGE_DOWN };
		static constexpr InputCode HOME{ GLFW_KEY_HOME };
		static constexpr InputCode END{ GLFW_KEY_END };
		static constexpr InputCode CAPS_LOCK{ GLFW_KEY_CAPS_LOCK };
		static constexpr InputCode SCROLL_LOCK{ GLFW_KEY_SCROLL_LOCK };
		static constexpr InputCode NUM_LOCK{ GLFW_KEY_NUM_LOCK };
		static constexpr InputCode PRINT_SCREEN{ GLFW_KEY_PRINT_SCREEN };
		static constexpr InputCode PAUSE{ GLFW_KEY_PAUSE };
		static constexpr InputCode F1{ GLFW_KEY_F1 };
		static constexpr InputCode F2{ GLFW_KEY_F2 };
		static constexpr InputCode F3{ GLFW_KEY_F3 };
		static constexpr InputCode F4{ GLFW_KEY_F4 };
		static constexpr InputCode F5{ GLFW_KEY_F5 };
		static constexpr InputCode F6{ GLFW_KEY_F6 };
		static constexpr InputCode F7{ GLFW_KEY_F7 };
		static constexpr InputCode F8{ GLFW_KEY_F8 };
		static constexpr InputCode F9{ GLFW_KEY_F9 };
		static constexpr InputCode F10{ GLFW_KEY_F10 };
		static constexpr InputCode F11{ GLFW_KEY_F11 };
		static constexpr InputCode F12{ GLFW_KEY_F12 };
		static constexpr InputCode F13{ GLFW_KEY_F13 };
		static constexpr InputCode F14{ GLFW_KEY_F14 };
		static constexpr InputCode F15{ GLFW_KEY_F15 };
		static constexpr InputCode F16{ GLFW_KEY_F16 };
		static constexpr InputCode F17{ GLFW_KEY_F17 };
		static constexpr InputCode F18{ GLFW_KEY_F18 };
		static constexpr InputCode F19{ GLFW_KEY_F19 };
		static constexpr InputCode F20{ GLFW_KEY_F20 };
		static constexpr InputCode F21{ GLFW_KEY_F21 };
		static constexpr InputCode F22{ GLFW_KEY_F22 };
		static constexpr InputCode F23{ GLFW_KEY_F23 };
		static constexpr InputCode F24{ GLFW_KEY_F24 };
		static constexpr InputCode F25{ GLFW_KEY_F25 };
		static constexpr InputCode NUMPAD_0{ GLFW_KEY_KP_0 };
		static constexpr InputCode NUMPAD_1{ GLFW_KEY_KP_1 };
		static constexpr InputCode NUMPAD_2{ GLFW_KEY_KP_2 };
		static constexpr InputCode NUMPAD_3{ GLFW_KEY_KP_3 };
		static constexpr InputCode NUMPAD_4{ GLFW_KEY_KP_4 };
		static constexpr InputCode NUMPAD_5{ GLFW_KEY_KP_5 };
		static constexpr InputCode NUMPAD_6{ GLFW_KEY_KP_6 };
		static constexpr InputCode NUMPAD_7{ GLFW_KEY_KP_7 };
		static constexpr InputCode NUMPAD_8{ GLFW_KEY_KP_8 };
		static constexpr InputCode NUMPAD_9{ GLFW_KEY_KP_9 };
		static constexpr InputCode NUMPAD_DECIMAL{ GLFW_KEY_KP_DECIMAL };
		static constexpr InputCode NUMPAD_DIVIDE{ GLFW_KEY_KP_DIVIDE };
		static constexpr InputCode NUMPAD_MULTIPLY{ GLFW_KEY_KP_MULTIPLY };
		static constexpr InputCode NUMPAD_SUBTRACT{ GLFW_KEY_KP_SUBTRACT };
		static constexpr InputCode NUMPAD_ADD{ GLFW_KEY_KP_ADD };
		static constexpr InputCode NUMPAD_ENTER{ GLFW_KEY_KP_ENTER };
		static constexpr InputCode NUMPAD_EQUAL{ GLFW_KEY_KP_EQUAL };
		static constexpr InputCode LEFT_SHIFT{ GLFW_KEY_LEFT_SHIFT };
		static constexpr InputCode LEFT_CONTROL{ GLFW_KEY_LEFT_CONTROL };
		static constexpr InputCode LEFT_ALT{ GLFW_KEY_LEFT_ALT };
		static constexpr InputCode LEFT_SUPER{ GLFW_KEY_LEFT_SUPER };
		static constexpr InputCode RIGHT_SHIFT{ GLFW_KEY_RIGHT_SHIFT };
		static constexpr InputCode RIGHT_CONTROL{ GLFW_KEY_RIGHT_CONTROL };
		static constexpr InputCode RIGHT_ALT{ GLFW_KEY_RIGHT_ALT };
		static constexpr InputCode RIGHT_SUPER{ GLFW_KEY_RIGHT_SUPER };
		static constexpr InputCode MENU{ GLFW_KEY_MENU };
	};

	namespace ManifestMouseCodes
	{
		static constexpr InputCode BUTTON_1{ GLFW_MOUSE_BUTTON_1 };
		static constexpr InputCode BUTTON_2{ GLFW_MOUSE_BUTTON_2 };
		static constexpr InputCode BUTTON_3{ GLFW_MOUSE_BUTTON_3 };
		static constexpr InputCode BUTTON_4{ GLFW_MOUSE_BUTTON_4 };
		static constexpr InputCode BUTTON_5{ GLFW_MOUSE_BUTTON_5 };
		static constexpr InputCode BUTTON_6{ GLFW_MOUSE_BUTTON_6 };
		static constexpr InputCode BUTTON_7{ GLFW_MOUSE_BUTTON_7 };
		static constexpr InputCode BUTTON_8{ GLFW_MOUSE_BUTTON_8 };
		static constexpr InputCode BUTTON_LAST{ GLFW_MOUSE_BUTTON_LAST };
		static constexpr InputCode BUTTON_LEFT{ GLFW_MOUSE_BUTTON_LEFT };
		static constexpr InputCode BUTTON_RIGHT{ GLFW_MOUSE_BUTTON_RIGHT };
		static constexpr InputCode BUTTON_MIDDLE{ GLFW_MOUSE_BUTTON_MIDDLE };
	}
}