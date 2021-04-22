#pragma once

void drawCube();
void drawPlane();
void drawQuad();
void parseOBJ(const char* filename, unsigned int& objVAO, unsigned int& numVertices);
void drawObj(unsigned int objVAO, unsigned int numVertices);
void drawScene(Shader &shader);