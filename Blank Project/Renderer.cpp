#include "Renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {	// RENDERER CODE FROM T8 (TERRAIN)

	heightmapShader = new ComputeShader("testing3.glsl");
	heightmapTex = SOIL_load_OGL_texture(TEXTUREDIR"gradient2.png", 1, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(heightmapTex, true);

	heightMap = new ComputeShaderTerrain(heightmapShader);
	camera = new Camera(-40, 270, Vector3());

	Vector3 dimensions = heightMap->GetTerrainSize();
	camera->SetPosition(dimensions * Vector3(0.5f, 2, 0.5f));

	shader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");

	if (!shader->LoadSuccess()) return;

	terrainTex = SOIL_load_OGL_texture(TEXTUREDIR"/Coursework/soil_ground.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	if (!terrainTex) return;

	SetTextureRepeating(terrainTex, true);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	init = true;


	root = new SceneNode();	// SCENE GRAPH FUNCTIONALITY

	SceneNode* terrain = new SceneNode();
	terrain->SetMesh(heightMap);
	root->AddChild(terrain);


	cubeMap = SOIL_load_OGL_cubemap(	// SKYBOX FUNCTIONALITY
		TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	skyboxShader = new Shader("SkyboxVertex.glsl", "SkyboxFragment.glsl");
	if (!skyboxShader->LoadSuccess()) return;

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	quad = Mesh::GenerateQuad();






	

	//WriteToTexture();
	//ReadFromTexture();
}

Renderer::~Renderer(void) {
	delete root;
	delete shader;
	delete camera;
	delete heightMap;

	delete quad;
	delete skyboxShader;

	delete heightmapShader;
	//glDeleteTextures(1, &heightmapTex);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	root->Update(dt);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();


	BindShader(shader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "apply"), 1);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "xCoord"), 4000.0f);
	glUniform1f(glGetUniformLocation(shader->GetProgram(), "zCoord"), 4500.0f);


	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTex);

	//heightMap->Draw();
	DrawNode(root);
}

void Renderer::DrawNode(SceneNode* n) {
	if (n->GetMesh()) {
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, model.values);

		glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());

		glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), 0);

		n->Draw(*this);
	}

	for (vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i) {
		DrawNode(*i);
	}
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();
	glDepthMask(GL_TRUE);
}







/*void Renderer::ReadFromTexture() {
	glBindTexture(GL_TEXTURE_2D, genTex);
	
	GLfloat* pixels = new GLfloat[256 * 256 * 4];

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);

	int w, h;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	std::cout << "size: " << w << " , " << h << std::endl;



	for (size_t x = 0; x < 256; ++x) {
		size_t y = 128;
		size_t elmes_per_line = 256 * 4;

		size_t row = y * elmes_per_line;
		size_t col = x * 4;

		GLfloat val = pixels[row + col];
		std::cout << val << " , " << pixels[row + col + 1] << " , " << pixels[row + col + 2] << " , " << pixels[row + col + 3] << std::endl;

	}
}

void Renderer::WriteToTexture() {
	glGenTextures(1, &genTex);
	glBindTexture(GL_TEXTURE_2D, genTex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 256, 256, 0, GL_RGBA, GL_FLOAT, 0);	// changed from GL_RGBA8 here + below
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glGenerateMipmap(GL_TEXTURE_2D);


	heightmapShader->Bind();

	int imageUnitIndex = 0;
	glUniform1i(glGetUniformLocation(heightmapShader->GetProgram(), "resultImage"), imageUnitIndex);
	glBindImageTexture(imageUnitIndex, genTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	//glUniform1f(glGetUniformLocation(heightmapShader->GetProgram(), "scale"), 0.45);
	//glUniform1f(glGetUniformLocation(heightmapShader->GetProgram(), "dims"), 256.0f);
	glUniform1f(glGetUniformLocation(heightmapShader->GetProgram(), "amplitude"), 500.0f);
	glUniform1f(glGetUniformLocation(heightmapShader->GetProgram(), "frequency"), 0.1f);
	glUniform1i(glGetUniformLocation(heightmapShader->GetProgram(), "octaves"), 8);
	glUniform1f(glGetUniformLocation(heightmapShader->GetProgram(), "persistence"), 0.8f);
	glUniform1f(glGetUniformLocation(heightmapShader->GetProgram(), "lacunarity"), 1.9f);
	glUniform2f(glGetUniformLocation(heightmapShader->GetProgram(), "offset"), 0.0f, 0.0f);

	//glUniform1iv(glGetUniformLocation(heightmapShader->GetProgram(), "perm"), 512, permValues);

	heightmapShader->Dispatch(256 / 16, 256 / 16, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	heightmapShader->Unbind();
}*/







/*void saveImage(const std::string& filename, int width, int height, const std::vector<float>& data) {
	std::ofstream file(filename, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Failed to open file for writing: " << filename << std::endl;
		return;
	}

	file << "P6\n" << width << " " << height << "\n255\n";

	for (size_t i = 0; i < data.size(); ++i) {
		unsigned char color = static_cast<unsigned char>(data[i] * 255.0f);
		file << color << color << color;
	}

	file.close();
}*/

void ImageSaving() {
	/*int widthtt, heighttt, channelstt;
	unsigned char* ht_map = SOIL_load_image
	(
		TEXTUREDIR"noise.png",
		&widthtt, &heighttt, &channelstt,
		SOIL_LOAD_L
	);

	int save_result = SOIL_save_image
	(
		TEXTUREDIR"noise.bmp",
		SOIL_SAVE_TYPE_BMP,
		widthtt, heighttt, channelstt,
		ht_map
	);
	//heightmapShader->Bind();


	//std::vector<unsigned char> imageData(512 * 512 * 4);
	/*unsigned char* imageData = new unsigned char[512 * 512 * 4];
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, imageData);

	if (SOIL_save_image(TEXTUREDIR"perlin_noise_test.bmp", SOIL_SAVE_TYPE_BMP, 512, 512, 4, imageData) == 0) {
		std::cerr << "Failed to save image" << std::endl;
	}*/

	/*std::vector<float> imageData(512 * 512 * 4);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, imageData.data());

	// Save image to file
	saveImage(TEXTUREDIR"perlin_noise.ppm", 512, 512, imageData);*/
}




