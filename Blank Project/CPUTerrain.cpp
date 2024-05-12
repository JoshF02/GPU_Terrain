#include "CPUTerrain.h"
#include <iostream>

CPUTerrain::CPUTerrain(int size) {
	int iWidth = size;
	int iHeight = size;
	Noise noise;

	numVertices = iWidth * iHeight;
	numIndices = (iWidth - 1) * (iHeight - 1) * 6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];

	Vector3 vertexScale = Vector3(16.0f, 1.0f, 16.0f);
	Vector2 textureScale = Vector2(1 / 16.0f, 1 / 16.0f);

	for (int z = 0; z < iHeight; ++z) {
		for (int x = 0; x < iWidth; ++x) {
			int offset = (z * iWidth) + x;

			float height = 0.0;
			float amplitude = 500.0;
			float frequency = 0.1f * ((float)size / 256);	// scales with terrain size
			int octaves = 8;
			float persistence = 0.8;
			float lacunarity = 1.9;
			float noiseOffset = 0;


			for (int i = 0; i <= octaves; i++) {
				Vector2 coord = Vector2((((float)x / iWidth) + noiseOffset) * frequency, (((float)z / iHeight) + noiseOffset) * frequency);
				height += noise.Simplex(coord.x, coord.y) * amplitude;
				amplitude *= persistence;
				frequency *= lacunarity;
			}

			vertices[offset] = Vector3(x, height, z) * vertexScale;
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
