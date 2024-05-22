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
			//a = pixels[row + col + 3];
			
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

	const int perm[256] = { 151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
							140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
							247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
							57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
							74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
							60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
							65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
							200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64,
							52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,
							207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213,
							119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
							129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104,
							218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241,
							81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,
							184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93,
							222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 };

	/*GLint maxVertexUniformComponents;
	glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_COMPONENTS, &maxVertexUniformComponents);
	std::cout << "Max vertex uniform components: " << maxVertexUniformComponents << std::endl;*/

	glUniform1iv(glGetUniformLocation(shader->GetProgram(), "perm256"), 256, perm);

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

	/*int w, h;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);*/

	//std::cout << "size: " << w << " , " << h << std::endl;
}