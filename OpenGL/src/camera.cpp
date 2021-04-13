#include "pch.h"
#include "camera.h"

Camera globalCamera;

glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(pos, pos + front, up);
}