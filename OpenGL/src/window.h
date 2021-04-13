#pragma once

#include <GLFW/glfw3.h>

#define SCR_WIDTH 1280
#define SCR_HEIGHT 720

extern glm::mat4 projectionMat;

GLFWwindow* initializeWindow();
bool loadGLFunctions();