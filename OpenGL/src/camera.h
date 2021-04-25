#pragma once

#include "glm.hpp"

struct Camera
{
	bool fpsMode = false;

	glm::vec3 pos = glm::vec3(0.0f, 2.0f, 3.0f);
	glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	float speed = 8.0f;
	float sensitivity = 0.2f;

	void cameraUpdate(float dt);
	glm::mat4 getViewMatrix();

	float yaw = -90.0f, pitch = 0.0f;
};

extern Camera globalCamera;