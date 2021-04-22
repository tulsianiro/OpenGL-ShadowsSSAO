#include "pch.h"
#include "draw.h"
#include "geometry.h"

void drawCube()
{
	static unsigned int cubeVAO = 0, cubeVBO = 0;
	if (!cubeVAO)
	{
		glGenVertexArrays(1, &cubeVAO); glGenBuffers(1, &cubeVBO);
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO); glBindVertexArray(cubeVAO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0); glEnableVertexAttribArray(1); glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36); // draw cube
	glBindVertexArray(0); // unbind VAO once done drawing
}

void drawPlane()
{
	static unsigned int planeVAO = 0, planeVBO = 0;
	if (!planeVAO)
	{
		glGenVertexArrays(1, &planeVAO); glGenBuffers(1, &planeVBO);
		glBindBuffer(GL_ARRAY_BUFFER, planeVBO); glBindVertexArray(planeVAO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0); glEnableVertexAttribArray(1); glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	}

	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6); // draw plane
	glBindVertexArray(0); // unbind VAO once done drawing
}

struct ObjVertex
{
	glm::vec3 P;
	glm::vec2 UV;
	glm::vec3 N;
};
void parseOBJ(const char* filename, unsigned int &objVAO, unsigned int &numVertices)
{
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> textures;
	std::vector<unsigned int> posIndex;
	std::vector<unsigned int> normIndex;
	std::vector<unsigned int> texIndex;

	std::string filenameMod = "models/" + std::string(filename);
	std::ifstream in(filenameMod);
	std::string line;

	while (std::getline(in, line))
	{
		std::string type = line.substr(0, 2);
		if (type == "v ")
		{
			std::string remainder = line.substr(2);
			std::stringstream v(remainder);
			float x, y, z;
			v >> x >> y >> z;
			vertices.push_back({ x, y, z });
		}
		else if (type == "vt")
		{
			std::string remainder = line.substr(3);
			std::stringstream v(remainder);
			float x, y;
			v >> x >> y;
			textures.push_back({ x, y });
		}
		else if (type == "vn")
		{
			std::string remainder = line.substr(3);
			std::stringstream v(remainder);
			float x, y, z;
			v >> x >> y >> z;
			normals.push_back({ x, y, z });
		}
		else if (type == "f ")
		{
			int p1, p2, p3, t1, t2, t3, n1, n2, n3;

			const char* cline = line.c_str();
			int ret = sscanf_s(cline, "f %d/%d/%d %d/%d/%d %d/%d/%d", &p1, &t1, &n1, &p2, &t2, &n2,
							 &p3, &t3, &n3);
			--p1; --p2; --p3; --t1; --t2; --t3; --n1; --n2; --n3;
			posIndex.push_back(p1); texIndex.push_back(t1); normIndex.push_back(n1);
			posIndex.push_back(p2); texIndex.push_back(t2); normIndex.push_back(n2);
			posIndex.push_back(p3); texIndex.push_back(t3); normIndex.push_back(n3);
		}
	}

	std::vector<ObjVertex> objectVertices;
	for (unsigned int i = 0; i < posIndex.size(); ++i)
	{
		ObjVertex objVert = { vertices[posIndex[i]], glm::vec2(0.0), normals[normIndex[i]] };
		objectVertices.push_back(objVert);
	}
	numVertices = objectVertices.size();

	unsigned int objVBO;
	// init and upload to VBO/VAO
	glGenBuffers(1, &objVBO);
	glBindBuffer(GL_ARRAY_BUFFER, objVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(ObjVertex) * objectVertices.size(), &objectVertices[0],
				 GL_STATIC_DRAW);
	glGenVertexArrays(1, &objVAO);
	glBindVertexArray(objVAO);
	glBindBuffer(GL_ARRAY_BUFFER, objVBO);

	// set vertex format (positions + normals) for rail
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ObjVertex),
						  (const void*)offsetof(ObjVertex, P));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ObjVertex),
						  (const void*)offsetof(ObjVertex, N));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ObjVertex),
						  (const void*)offsetof(ObjVertex, UV));
}

void drawObj(unsigned int objVAO, unsigned int numVertices)
{
	glBindVertexArray(objVAO);
	glDrawArrays(GL_TRIANGLES, 0, numVertices); // draw plane
	glBindVertexArray(0); // unbind VAO once done drawing
}

void drawScene(Shader &shader)
{
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);
	drawPlane();
	// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	drawCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.setMat4("model", model);
	drawCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 1.0f, 2.0));
	//model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.5));
	shader.setMat4("model", model);
}

void drawQuad()
{
	static unsigned int quadVAO = 0, quadVBO = 0;
	if (!quadVAO)
	{
		glGenVertexArrays(1, &quadVAO); glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO); glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0); glEnableVertexAttribArray(1);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}
