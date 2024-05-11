#pragma once
#include <string>
#include "../nclgl/Mesh.h"
#include "../nclgl/ComputeShader.h"

class ComputeShaderTerrain : public Mesh
{
public:
	ComputeShaderTerrain(ComputeShader* computeShader, int size);
	~ComputeShaderTerrain(void) {};

	Vector3 GetTerrainSize() const { return terrainSize; }

	void WriteToTexture();

	void ReadFromTexture();

protected:
	Vector3 terrainSize;

	GLuint tex;
	ComputeShader* shader;
	GLfloat* pixels;
	int size;
};
