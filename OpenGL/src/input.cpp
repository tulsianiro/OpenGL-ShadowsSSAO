#include "pch.h"
#include "input.h"
#include "camera.h"

extern Camera globalCamera;
static bool cameraChange = false;
extern bool useSSAO;
extern int showTexInt;

void processFrameInput(GLFWwindow *window, float dt)
{
	static bool first = true;

	if (globalCamera.fpsMode)
	{
		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			globalCamera.pos += dt * globalCamera.speed * globalCamera.front;
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			globalCamera.pos -= dt * globalCamera.speed * globalCamera.front;
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			globalCamera.pos -= (dt * glm::normalize(glm::cross(globalCamera.front, globalCamera.up)) *
								 globalCamera.speed);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			globalCamera.pos += (dt * glm::normalize(glm::cross(globalCamera.front, globalCamera.up)) *
								 globalCamera.speed);

		static double mouseX, mouseY;
		if (first)
		{
			int windowWidth, windowHeight;
			cameraChange = false;
			glfwGetWindowSize(window, &windowWidth, &windowHeight);
			glfwSetCursorPos(window, windowWidth / 2.0f, windowHeight / 2.0f);
			mouseX = (double)windowWidth / 2.0f;
			mouseY = (double)windowHeight / 2.0f;
			first = false;
		}
		else if (cameraChange)
		{
			cameraChange = false;
			glfwSetCursorPos(window, mouseX, mouseY);
		}

		double currX = 0.0f, currY = 0.0f;
		glfwGetCursorPos(window, &currX, &currY);
		double xOffset = currX - mouseX;
		double yOffset = mouseY - currY;
		mouseX = currX; mouseY = currY;

		xOffset *= globalCamera.sensitivity; yOffset *= globalCamera.sensitivity;

		// a lot of this math for the FPS cam was found on learnopengl.com
		globalCamera.yaw += (float)xOffset;
		globalCamera.pitch += (float)yOffset;
		globalCamera.pitch = glm::clamp(globalCamera.pitch, -89.0f, 89.0f);
		glm::vec3 direction;
		direction.x = cos(glm::radians(globalCamera.yaw)) * cos(glm::radians(globalCamera.pitch));
		direction.y = sin(glm::radians(globalCamera.pitch));
		direction.z = sin(glm::radians(globalCamera.yaw)) * cos(glm::radians(globalCamera.pitch));
		globalCamera.front = glm::normalize(direction);
	}
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_C && action == GLFW_PRESS)
	{
		if (globalCamera.fpsMode)
		{
			globalCamera.fpsMode = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
		else
		{
			globalCamera.fpsMode = true;
			cameraChange = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}
	
	if (key == GLFW_KEY_0 && action == GLFW_PRESS)
	{
		showTexInt = 0;
	}
	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		showTexInt = 1;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		showTexInt = 2;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		showTexInt = 3;
	}
	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		showTexInt = 4;
	}
	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		showTexInt = 5;
	}

	if (key == GLFW_KEY_X && action == GLFW_PRESS)
	{
		useSSAO = !useSSAO;
	}
}