#include "Renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {	// RENDERER CODE FROM T8 (TERRAIN)
	heightMap = new CPUTerrain();
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






	heightmapShader = new ComputeShader("testing3.glsl");
	heightmapTex = SOIL_load_OGL_texture(TEXTUREDIR"gradient2.png", 1, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(heightmapTex, true);

	WriteToTexture();
	ReadFromTexture();
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

/*const int WIDTH = 10;
const int HEIGHT = 10;
const int SIZETT = 10 * 10;

// Function to generate terrain heightmap using Perlin noise
void generateTerrainHeightmap(float frequency, float amplitude, int octaves, float heightmap[SIZETT]) {
	PerlinNoise noise; // Initialize FastNoise object

	int index = 0;
	for (int x = 0; x < WIDTH; ++x) {
		for (int y = 0; y < HEIGHT; ++y) {
			float noiseValue = 0.0f;
			float amp = amplitude;
			float freq = frequency;
			for (int o = 0; o < octaves; ++o) {
				noiseValue += noise.noise((float)x * freq, (float)y * freq) * amp;
				freq *= 2.0f; // Increase frequency for each octave
				amp *= 0.5f; // Decrease amplitude for each octave
			}
			heightmap[index++] = noiseValue;
		}
	}
}*/

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	root->Update(dt);

	/*PerlinNoise perlin;

	// Example usage:
	double x = 0.5775;
	double y = 0.9556;
	double noiseValue = perlin.noise(x, y);

	std::cout << "Perlin noise value at (" << x << ", " << y << "): " << noiseValue << std::endl;*/
	
	/*float* heightmap = new float[SIZETT];

	generateTerrainHeightmap(0.05f, 10.0f, 4, heightmap);

	// Output the heightmap (for demonstration purposes)
	for (int i = 0; i < SIZETT; ++i) {
		std::cout << heightmap[i] << " ";
		if ((i + 1) % WIDTH == 0) {
			std::cout << std::endl;
		}
	}*/

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







void Renderer::ReadFromTexture() {
	glBindTexture(GL_TEXTURE_2D, genTex);
	
	GLfloat* pixels = new GLfloat[256 * 256 * 4];

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);

	int w, h;
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);

	std::cout << "size: " << w << " , " << h << std::endl;



	for (size_t x = 0; x < 256; ++x) {
		/*GLuint r, g, b, a;

		size_t y = 128;
		size_t elmes_per_line = 256 * 4;

		size_t row = y * elmes_per_line;
		size_t col = x * 4;

		r = pixels[row + col];
		g = pixels[row + col + 1];
		b = pixels[row + col + 2];
		a = pixels[row + col + 3];

		GLuint gray = (GLuint)((r + g + b) / 3.0);
		std::cout << gray << std::endl;*/
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

	/*int permValues[] = {151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
						  140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
						  247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
						  57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
						  74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
						  60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
						  65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
						  200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3,
						  64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82,
						  85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223,
						  183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155,
						  167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224,
						  232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238,
						  210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
						  107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115,
						  121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29,
						  24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,

						151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225,
						  140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148,
						  247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32,
						  57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
						  74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
						  60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54,
						  65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169,
						  200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3,
						  64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82,
						  85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223,
						  183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155,
						  167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224,
						  232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238,
						  210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
						  107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115,
						  121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29,
						  24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
	};

	glUniform1iv(glGetUniformLocation(heightmapShader->GetProgram(), "perm"), 512, permValues);*/

	heightmapShader->Dispatch(256 / 16, 256 / 16, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	heightmapShader->Unbind();
}







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




