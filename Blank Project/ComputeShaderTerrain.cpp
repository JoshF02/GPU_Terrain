#include "ComputeShaderTerrain.h"
#include <iostream>

ComputeShaderTerrain::ComputeShaderTerrain(ComputeShader* computeShader, int size) {
	shader = computeShader;
	int iWidth = size;
	int iHeight = size;
	this->size = size;

	numVertices = iWidth * iHeight;
	numIndices = (iWidth - 1) * (iHeight - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];

	Vector3 vertexScale = Vector3(16.0f, 1.0f, 16.0f);
	Vector2 textureScale = Vector2(1 / 16.0f, 1 / 16.0f);

	WriteToTexture();
	ReadFromTexture();

	for (int z = 0; z < iHeight; ++z) {
		for (int x = 0; x < iWidth; ++x) {
			int offset = (z * iWidth) + x;

			GLfloat r, g, b, a;

			size_t elmes_per_line = size * 4;

			size_t row = z * elmes_per_line;
			size_t col = x * 4;

			r = pixels[row + col];
			g = pixels[row + col + 1];
			b = pixels[row + col + 2];
			a = pixels[row + col + 3];
			
			vertices[offset] = Vector3(r, g, b) * vertexScale;
			textureCoords[offset] = Vector2(x, z) * textureScale;
		}
	}

	int i = 0;

	for (int z = 0; z < iHeight - 1; ++z) {
		for (int x = 0; x < iWidth - 1; ++x) {
			int a = (z * (iWidth)) + x;
			int b = (z * (iWidth)) + (x + 1);
			int c = ((z + 1) * (iWidth)) + (x + 1);
			int d = ((z + 1) * (iWidth)) + x;

			indices[i++] = a;
			indices[i++] = c;
			indices[i++] = b;

			indices[i++] = c;
			indices[i++] = a;
			indices[i++] = d;
		}
	}
	GenerateNormals();
	GenerateTangents();
	BufferData();

	terrainSize.x = vertexScale.x * (iWidth - 1);
	terrainSize.y = vertexScale.y * 255.0f;
	terrainSize.z = vertexScale.z * (iHeight - 1);
}

void ComputeShaderTerrain::WriteToTexture() {
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, size, size, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);


	shader->Bind();

	int imageUnitIndex = 0;
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "resultImage"), imageUnitIndex);
	glBindImageTexture(imageUnitIndex, tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glUniform1f(glGetUniformLocation(shader->GetProgram(), "amplitude"), 500.0f);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "frequency"), 0.1f * ((float)size / 256));
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "octaves"), 8);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "persistence"), 0.8f);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "lacunarity"), 1.9f);
	glUniform2f(glGetUniformLocation(shader->GetProgram(), "offset"), 0.0f, 0.0f);
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "size"), size);

	shader->Dispatch(size / 16, size / 16, 1);			// experiment with different layout sizes
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	shader->Unbind();
}

void ComputeShaderTerrain::ReadFromTexture() {
	glBindTexture(GL_TEXTURE_2D, tex);

	pixels = new GLfloat[size * size * 4];

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);

	int w, h;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	//std::cout << "size: " << w << " , " << h << std::endl;
}