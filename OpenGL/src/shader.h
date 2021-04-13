#pragma once

#include <string>

struct Shader
{
	struct ShaderSources
	{
		std::string vertexSource;
		std::string fragmentSource;
	};

	enum class ShaderType
	{
		VERTEX = 0,
		FRAGMENT,
		NONE
	};

	unsigned int programID;

	Shader(std::string filePath);
	void use();
	
	// set uniform variables
	void setBool(std::string name, bool val);
	void setInt(std::string name, int val);
	void setFloat(std::string name, float val);
	void setVec2(std::string name, glm::vec2 val);
	void setVec2(std::string name, float x, float y);
    void setVec3(std::string name, glm::vec3& val);
    void setVec3(std::string name, float x, float y, float z);
    void setVec4(std::string name, glm::vec4& val);
    void setVec4(std::string name, float x, float y, float z, float w);
    void setMat4(std::string name, glm::mat4& val);

private:
	void parseShader(std::string filePath, ShaderSources& outSource);
	unsigned int compileShader(std::string& shaderSource, unsigned int type);
};