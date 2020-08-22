#include "pcheader.h"
#include "OverEngine/Input/Input.h"

#include "OverEngine/Core/Application.h"
#include <GLFW/glfw3.h>

namespace OverEngine
{
	bool Input::IsKeyPressed(KeyCode keycode)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow());
		auto state = glfwGetKey(window, (int)keycode);
		return state == (int)KeyTrigger::Press;
	}

	bool Input::IsMouseButtonPressed(KeyCode button)
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow());
		auto state = glfwGetMouseButton(window, (int)button);
		return state == (int)KeyTrigger::Press;
	}

	Vector2 Input::GetMousePosition()
	{
		auto window = static_cast<GLFWwindow*>(Application::Get().GetMainWindow().GetNativeWindow());
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		return { (float)xpos, (float)ypos };
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().y;
	}
}