#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Camera.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/ComputeShader.h"
#include "CPUTerrain.h"
#include "ComputeShaderTerrain.h"

class Renderer : public OGLRenderer	{
public:
	Renderer(Window &parent);
	 ~Renderer(void);

	 void RenderScene()				override;
	 void UpdateScene(float msec)	override;

protected:
	void DrawNode(SceneNode* n);

	SceneNode* root;
	Camera* camera;
	Shader* shader;
	ComputeShaderTerrain* heightMap;
	GLuint terrainTex;

	void DrawSkybox();

	//void ReadFromTexture();
	//void WriteToTexture();
	ComputeShader* heightmapShader;
	GLuint heightmapTex;
	GLuint genTex;

	Shader* skyboxShader;
	GLuint cubeMap;
	Mesh* quad;

};

