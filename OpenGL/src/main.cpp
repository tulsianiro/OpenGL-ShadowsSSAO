#include "pch.h"
#include "shader.h"
#include "draw.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

struct GBuffer
{
	unsigned int gBuffer, gPosition, gNormal, gAlbedo;
	unsigned int gLightSpacePosition;
};

struct GrayFBO
{
	unsigned int FBO, colorBuffer;
};

unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

GBuffer generateGBuffer()
{
	unsigned int gBuffer, gPosition, gNormal, gAlbedo;
	unsigned int gLightSpacePosition;
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

	glGenTextures(1, &gLightSpacePosition); glBindTexture(GL_TEXTURE_2D, gLightSpacePosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gLightSpacePosition, 0);

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
	unsigned int attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
	glDrawBuffers(4, attachments); // we will draw into these 4 attachments when rendering with gBuffer FBO
	unsigned int rboDepth; // we want a depth attachment, but don't need it as a texture
	glGenRenderbuffers(1, &rboDepth); glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT); // alloc
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind FBO

	return { gBuffer, gPosition, gNormal, gAlbedo, gLightSpacePosition };
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

GrayFBO generateShadowFBO(unsigned int shadowWidth, unsigned int shadowHeight)
{
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return { depthMapFBO, depthMap };
}

std::vector<glm::vec3> generateSSAOKernel()
{
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	std::default_random_engine generator;

	std::vector<glm::vec3> ssaoKernel;
	for (unsigned int i = 0; i < 64; ++i)
	{
		// generate a direction vector that precludes the bottom half of a sphere
		glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
		sample = glm::normalize(sample); // normalize it since it is a direction
		sample *= randomFloats(generator); // multiply by a float between 0 and 1 to get a magnitude 
		float scale = float(i) / 64.0;

		// the above uniformly distributes our sample kernel, so we decrease the scaling factor such that 
		// it accelerates towards the center as i increases.
		scale = lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		ssaoKernel.push_back(sample);
	}

	return ssaoKernel;
}

unsigned int generateSSAONoiseTexture()
{
	std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0);
	std::default_random_engine generator;

	std::vector<glm::vec3> ssaoNoise;
	// create 16 rotations that will be tiled across our pixels
	for (unsigned int i = 0; i < 16; i++)
	{
		// rotate around z-axis (in tangent space). values between -1 and 1.
		// direction vectors in tangent space, oriented about the normal
		glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); 
		ssaoNoise.push_back(noise);
	}
	unsigned int noiseTexture; glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	// 4x4 texture that is GL_REPEAT e.g. tiled.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	return noiseTexture;
}

bool useSSAO = true;

int main()
{
	GLFWwindow* window = initializeWindow(); if (!window) return -1;
	bool success = loadGLFunctions(); if (!success) return -1;
	glfwSetKeyCallback(window, keyCallback); glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	float dt = 0.0f, lastFrame = 0.0f;

	Shader gBufferShader("ssao_gbuffer.shader");
	Shader ssaoShader("ssao_main.shader");
	Shader blurShader("ssao_blur.shader");
	Shader deferredShader("deferred_light.shader");
	Shader depthShader("depth_map.shader");
	Shader quadShader("depth_quad.shader");

	GBuffer gBuffer = generateGBuffer(); // used for deferred shading
	GrayFBO ssaoFBO = generateGrayFBO(); // used for calculating ssao texture
	GrayFBO ssaoBlur = generateGrayFBO(); // used for blurring the texture to de-noise

	unsigned int SHADOW_WIDTH = 12000, SHADOW_HEIGHT = 12000;
	GrayFBO shadowFBO = generateShadowFBO(SHADOW_WIDTH, SHADOW_HEIGHT);

	unsigned int normalMap = loadTexture("normal_map.png");

	// generate sample kernel to pass into SSAO shader
	std::vector<glm::vec3> ssaoKernel = generateSSAOKernel();
	unsigned int noiseTexture = generateSSAONoiseTexture();
	
	// light constants
	glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);
	glm::vec3 lightColor(0.3);
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	//lightProjection = glm::perspective(glm::radians(45.0f), (GLfloat)SHADOW_WIDTH / (GLfloat)SHADOW_HEIGHT, near_plane, far_plane); // note that if you use a perspective projection matrix you'll have to change the light position as the current light position isn't enough to reflect the whole scene
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	depthShader.use();
	depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

	/*
		GBUFFER CONSTANT UNIFORMS
	*/
	gBufferShader.use();
	gBufferShader.setMat4("projection", projectionMat);
	gBufferShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
	gBufferShader.setInt("invertedNormals", 0);
	gBufferShader.setInt("normalMap", 0);
	/*
		SSAO CONSTANT UNIFORMS
	*/
	ssaoShader.use();
	for (unsigned int i = 0; i < 64; ++i)
		ssaoShader.setVec3("samples[" + std::to_string(i) + "]", ssaoKernel[i]);
	ssaoShader.setMat4("projection", projectionMat);
	ssaoShader.setInt("gPosition", 0);
	ssaoShader.setInt("gNormal", 1);
	ssaoShader.setInt("texNoise", 2);
	/*
		BLUR CONSTANT UNIFORMS
	*/
	blurShader.use();
	blurShader.setInt("ssaoInput", 0);
	/*
		DEFERRED LIGHT CONSTANT UNIFORMS
	*/
	deferredShader.use();
	deferredShader.setInt("gPosition", 0);
	deferredShader.setInt("gNormal", 1);
	deferredShader.setInt("gAlbedo", 2);
	deferredShader.setInt("ssao", 3);
	deferredShader.setInt("gLightSpacePosition", 4);
	deferredShader.setInt("shadowMap", 5);

	unsigned int objVAO, numVertices;
	parseOBJ("teapot.obj", objVAO, numVertices);

	unsigned int obj2VAO, numVertices2;
	parseOBJ("backpack.obj", obj2VAO, numVertices2);

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = glfwGetTime();
		dt = currentFrame - lastFrame;
		lastFrame = currentFrame;
		processFrameInput(window, dt);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glm::mat4 viewMat = globalCamera.getViewMatrix();
		glm::mat4 model = glm::mat4(1.0f);

		depthShader.use();
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO.FBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		drawScene(depthShader);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.4f, 3.0f, 1.0));
		//model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::scale(model, glm::vec3(2.0));
		depthShader.setMat4("model", model);
		drawObj(objVAO, numVertices);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-2.0f, 1.6f, 2.0));
		model = glm::rotate(model, glm::radians(180.0f), glm::normalize(glm::vec3(1.0, 1.0, 1.0)));
		model = glm::scale(model, glm::vec3(0.75));
		depthShader.setMat4("model", model);
		drawObj(obj2VAO, numVertices2);


		// draw depth texture to screen
		/*
		glViewport(0, 0, 1280, 720);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		quadShader.use();
		drawQuad();
		*/

		// geometry
		
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		gBufferShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, normalMap);
		gBufferShader.setMat4("view", viewMat);	
		drawScene(gBufferShader);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.4f, 3.0f, 1.0));
		//model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
		model = glm::scale(model, glm::vec3(2.0));
		gBufferShader.setMat4("model", model);
		drawObj(objVAO, numVertices);
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(-2.0f, 1.6f, 2.0));
		model = glm::rotate(model, glm::radians(180.0f), glm::normalize(glm::vec3(1.0, 1.0, 1.0)));
		model = glm::scale(model, glm::vec3(0.75));
		gBufferShader.setMat4("model", model);
		drawObj(obj2VAO, numVertices2);

		// ssao
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO.FBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ssaoShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, noiseTexture);
		drawQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// blur
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlur.FBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blurShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoFBO.colorBuffer);
		drawQuad();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// lighting
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		deferredShader.use();
		// send light relevant uniforms
		glm::vec3 lightPosView = glm::vec3(viewMat * glm::vec4(lightPos, 1.0));
		deferredShader.setVec3("light.Position", lightPosView);
		deferredShader.setVec3("light.Color", lightColor);
		deferredShader.setBool("useSSAO", useSSAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gBuffer.gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gBuffer.gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gBuffer.gAlbedo);
		glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
		glBindTexture(GL_TEXTURE_2D, ssaoBlur.colorBuffer);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, gBuffer.gLightSpacePosition);
		glActiveTexture(GL_TEXTURE5);
		glBindTexture(GL_TEXTURE_2D, shadowFBO.colorBuffer);
		drawQuad();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}