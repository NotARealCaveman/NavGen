#pragma once

//#include <glad/glad.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Input.h"

namespace Manifest_Core
{
	MFbool IsKeyActioned(const InputCode& keyCode,const InputCode& keyAction, GLFWwindow* window);
}