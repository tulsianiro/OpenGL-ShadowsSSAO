#include "pch.h"
#include "shader.h"

void Shader::parseShader(std::string filePath, ShaderSources& outSource)
{
	ShaderType currType = ShaderType::NONE;

	std::ifstream fileStream("shaders/" + filePath);
	std::stringstream shaderStream[(int)ShaderType::NONE];

	std::string line;
	while (std::getline(fileStream, line))
	{
		if (line.find("#shader") != std::string::npos &&
			line.find("vertex") != std::string::npos)
		{
			currType = ShaderType::VERTEX;
		}
		else if (line.find("#shader") != std::string::npos &&
				 line.find("fragment") != std::string::npos)
		{
			currType = ShaderType::FRAGMENT;
		}
		else
		{
			shaderStream[(int)currType] << line << "\n";
		}
	}

	outSource.vertexSource = shaderStream[(int)ShaderType::VERTEX].str();
	outSource.fragmentSource = shaderStream[(int)ShaderType::FRAGMENT].str();
}

unsigned int Shader::compileShader(std::string& shaderSource, unsigned int type)
{
	unsigned int shaderID = glCreateShader(type);
	const char* srcPointer = shaderSource.c_str();
	glShaderSource(shaderID, 1, &srcPointer, 0);
	glCompileShader(shaderID);

	int result;
	glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE)
	{
		char message[512];
		glGetShaderInfoLog(shaderID, 512, 0, message);

		std::string shaderType = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
		std::cout << "FAILED TO COMPILE " << shaderType << " SHADER!" << std::endl;
		std::cout << message << std::endl;
		return 0;
	}

	return shaderID;
}

Shader::Shader(std::string filePath)
{
	ShaderSources shaderSource;
	parseShader(filePath, shaderSource);

	unsigned int programID = glCreateProgram();
	unsigned int VS = compileShader(shaderSource.vertexSource, GL_VERTEX_SHADER);
	unsigned int FS = compileShader(shaderSource.fragmentSource, GL_FRAGMENT_SHADER);

	glAttachShader(programID, VS);
	glAttachShader(programID, FS);
	glLinkProgram(programID);
	glValidateProgram(programID);

	// cleanup shaders once linked
	glDetachShader(programID, VS);
	glDetachShader(programID, FS);
	glDeleteShader(VS);
	glDeleteShader(FS);

	this->programID = programID;
}

void Shader::use()
{
	glUseProgram(programID);
}

void Shader::setBool(std::string name, bool val)
{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)val);
}

void Shader::setInt(std::string name, int val)
{
	glUniform1i(glGetUniformLocation(programID, name.c_str()), val);
}

void Shader::setFloat(std::string name, float val)
{
	glUniform1f(glGetUniformLocation(programID, name.c_str()), val);
}

void Shader::setVec2(std::string name, glm::vec2 val)
{
	glUniform2fv(glGetUniformLocation(programID, name.c_str()), 1, &val[0]);
}
void Shader::setVec2(std::string name, float x, float y)
{
	glUniform2f(glGetUniformLocation(programID, name.c_str()), x, y);
}
void Shader::setVec3(std::string name, glm::vec3& val)
{
	glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &val[0]);
}
void Shader::setVec3(std::string name, float x, float y, float z)
{
	glUniform3f(glGetUniformLocation(programID, name.c_str()), x, y, z);
}
void Shader::setVec4(std::string name, glm::vec4& val)
{
	glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &val[0]);
}
void Shader::setVec4(std::string name, float x, float y, float z, float w)
{
	glUniform4f(glGetUniformLocation(programID, name.c_str()), x, y, z, w);
}
void Shader::setMat4(std::string name, glm::mat4& val)
{
	glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &val[0][0]);
}


