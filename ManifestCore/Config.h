#pragma once
#include <iostream>

//#include <glad/glad.h>
#include <glad/gl.h>
#include <GLFW/glfw3.h>


#include <ManifestUtility/Typenames.h>

using namespace Manifest_Utility;


namespace Manifest_Core
{			
	//glofried wrapper for callbacks, contains game reference
	class Framework;
	struct GLFWCallbacks
	{
		GLFWCallbacks(Framework* _game);
		//used when a key is physically pressed
		static void GLKeypressCallback(GLFWwindow* window, MFint32 keycode, MFint32 scancode, MFint32 action, MFint32 modifier);
		static void GLWindowResizeCallback(GLFWwindow* window, MFint32 w, MFint32 h);
		static void GLScrollCallback(GLFWwindow* window, MFdouble x, MFdouble y);
		static void GLErrorCallback(GLint error, const MFstring desc);
		static void GLWindowFocusCallback(GLFWwindow* window, GLint focused);		

		static Framework* game;
	};
}

