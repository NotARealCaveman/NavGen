#include "InputSupport.h"

using namespace Manifest_Core;

MFbool Manifest_Core::IsKeyActioned(const InputCode& keyCode, const InputCode& keyAction, GLFWwindow* window)
{	
	return glfwGetKey(window, keyCode) == keyAction;
}