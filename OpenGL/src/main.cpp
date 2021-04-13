#include "pch.h"
#include "shader.h"
#include "draw.h"

struct GBuffer
{
	unsigned int gBuffer, gPosition, gNormal, gAlbedo;
};

struct GrayFBO
{
	unsigned int FBO, colorBuffer;
};

GBuffer generateGBuffer()
{
	unsigned int gBuffer, gPosition, gNormal, gAlbedo; 
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	// position color buffer
	glGenTextures(1, &gPosition); glBindTexture(GL_TEXTURE_2D, gPosition);
	// create empty texture. with 16 bit float rgba, size width/height.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	// no interpolation since we are reading these values in shaders
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	// output color 0 of framebuffer goes into gPosition texture 
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
	// normal color buffer
	glGenTextures(1, &gNormal); glBindTexture(GL_TEXTURE_2D, gNormal); // same as above
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// output color 1 of framebuffer goes into gNormal texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
	// store albedo color, which in this case is just going to be 1 color for simplicity
	glGenTextures(1, &gAlbedo);
	glBindTexture(GL_TEXTURE_2D, gAlbedo);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// output color 2 of framebuffer goes into gAlbedo texture
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments); // we will draw into these 3 attachments when rendering with gBuffer FBO
	unsigned int rboDepth; // we want a depth attachment, but don't need it as a texture
	glGenRenderbuffers(1, &rboDepth); glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT); // alloc
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind FBO

	return { gBuffer, gPosition, gNormal, gAlbedo };
}

GrayFBO generateGrayFBO()
{
	unsigned int FBO; glGenFramebuffers(1, &FBO);  
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	unsigned colorBuffer; glGenTextures(1, &colorBuffer);	
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	// create empty texture with only a single channel
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind FBO

	return { FBO, colorBuffer };
}


int main()
{
	GLFWwindow* window = initializeWindow(); if (!window) return -1;
	bool success = loadGLFunctions(); if (!success) return -1;
	glfwSetKeyCallback(window, keyCallback); glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	float dt = 0.0f, lastFrame = 0.0f;

	Shader gBufferShader("ssao_gbuffer.shader");

	GBuffer gBuffer = generateGBuffer(); // used for deferred shading
	GrayFBO ssaoFBO = generateGrayFBO(); // used for calculating ssao texture
	GrayFBO ssaoBlur = generateGrayFBO(); // used for blurring the texture to de-noise

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processFrameInput(window, dt);

		glm::mat4 viewMat = globalCamera.getViewMatrix();
		// projection matrix stored as projectionMat globally

		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gBufferShader.use();
		gBufferShader.setMat4("projection", projectionMat);
		gBufferShader.setMat4("view", viewMat);	
		gBufferShader.setInt("invertedNormals", 0);
		drawScene(gBufferShader);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}