#define _CRT_SECURE_NO_DEPRECATE
#include "rt3d.h"
#include "rt3dObjLoader.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/noise.hpp>
#include <stack>
#include <iostream>
#include "PNGProcessor.h"

using namespace std;

#if _DEBUG
#pragma comment(linker, "/subsystem:\"console\" /entry:\"WinMainCRTStartup\"")
#endif
#define DEG_TO_RADIAN 0.017453293

// Mesh Index Count
GLuint meshIndexCount = 0;
GLuint meshObjects[1];

// Shader Programs
GLuint skyboxProgram;
GLuint furProgram;

// Fur Settings
float furLength = 0.2;
int layers = 30;
int furDensity = 50000;
int furPatternNum = 0;
float furFlowOffset = 0; // For fur animation/ movement.
bool increment = false;

// Camera Properties
glm::vec3 eye(0.0f, 1.0f, 3.0f);
glm::vec3 at(0.0f, 1.0f, 2.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

GLfloat r = 0.0f;
GLfloat lr = 0.0f;

// Matrix Stack
stack<glm::mat4> mvStack;

// Skybox Images
GLuint textures[6];
GLuint skybox[5];

// Set up rendering context
SDL_Window * setupRC(SDL_GLContext &context) {
	SDL_Window * window;
	if (SDL_Init(SDL_INIT_VIDEO) < 0) // Initialize video
		rt3d::exitFatalError("Unable to initialize SDL");

	// Request an OpenGL 3.0 context.

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);  // double buffering on
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8); // 8 bit alpha buffering
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4); // Turn on x4 multisampling anti-aliasing (MSAA)

	// Create 800x600 window
	window = SDL_CreateWindow("SDL/GLM/OpenGL Demo", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (!window) // Check window was created OK
		rt3d::exitFatalError("Unable to create window");

	context = SDL_GL_CreateContext(window); // Create opengl context and attach to window
	SDL_GL_SetSwapInterval(1); // set swap buffers to sync with monitor's vertical refresh rate
	return window;
}

// A simple texture loading function
// lots of room for improvement - and better error checking!
GLuint loadBitmap(const char *fname) {
	GLuint texID;
	glGenTextures(1, &texID); // generate texture ID

	// load file - using core SDL library
	SDL_Surface *tmpSurface;
	tmpSurface = SDL_LoadBMP(fname);
	if (!tmpSurface) {
		std::cout << "Error loading bitmap" << std::endl;
	}

	// bind texture and set parameters
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	SDL_PixelFormat *format = tmpSurface->format;

	GLuint externalFormat, internalFormat;
	if (format->Amask) {
		internalFormat = GL_RGBA;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGBA : GL_BGRA;
	}
	else {
		internalFormat = GL_RGB;
		externalFormat = (format->Rmask < format->Bmask) ? GL_RGB : GL_BGR;
	}

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, tmpSurface->w, tmpSurface->h, 0,
		externalFormat, GL_UNSIGNED_BYTE, tmpSurface->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);

	SDL_FreeSurface(tmpSurface); // texture loaded, free the temporary buffer
	return texID;	// return value of texture ID
}
GLuint loadCubeMap(const char*fname[6], GLuint *texID)
{
	glGenTextures(1, texID); // generate texture ID
	GLenum sides[6] = { GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
						GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
						GL_TEXTURE_CUBE_MAP_POSITIVE_X,
						GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
						GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
						GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
	};
	SDL_Surface *tmpsurface;

	glBindTexture(GL_TEXTURE_CUBE_MAP, *texID); // bind texture and set parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (int i = 0; i < 6;i++)
	{
		// loadfile - using core SDL Library
		tmpsurface = SDL_LoadBMP(fname[i]);
		if (!tmpsurface)
		{
			std::cout << "Cubemap: Error loading bitmap" << std::endl;
			return *texID;
		}

		glTexImage2D(sides[i], 0, GL_RGB, tmpsurface->w, tmpsurface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, tmpsurface->pixels);
		//texture oaded, free the temporary buffer
		SDL_FreeSurface(tmpsurface);
	}
	return *texID; // return value of texture ID, redundant really
}

void shaderInit(void) {
	// Skybox Shader Program
	skyboxProgram = rt3d::initShaders("skyboxShader.vert", "skyboxShader.frag");

	// Fur Shader Program
	furProgram = rt3d::initShaders("furShader.vert", "furShader.frag");
	GLuint uniformIndex = glGetUniformLocation(furProgram, "textureUnit1");
	glUniform1i(uniformIndex, 1);
	uniformIndex = glGetUniformLocation(furProgram, "textureUnit0");
	glUniform1i(uniformIndex, 0);
}

void init(void) {
	shaderInit();

	// Load Skybox
	const char *cubeTexFiles[6] = {
		"Skybox/skyrender_left.bmp", "Skybox/skyrender_front.bmp",
		"Skybox/skyrender_right.bmp", "Skybox/skyrender_back.bmp",
		"Skybox/skyrender_top.bmp", "Skybox/skyrender_bottom.bmp"
	};
	loadCubeMap(cubeTexFiles, &skybox[0]);

	vector<GLfloat> verts;
	vector<GLfloat> norms;
	vector<GLfloat> tex_coords;
	vector<GLuint> indices;

	// Prepare Cube model
	rt3d::loadObj("wolf.obj", verts, norms, tex_coords, indices);
	meshIndexCount = indices.size();
	meshObjects[0] = rt3d::createMesh(verts.size() / 3, verts.data(), nullptr, norms.data(), tex_coords.data(), meshIndexCount, indices.data());

	// Load Textures
	textures[0] = loadBitmap("tiger.bmp");
	textures[1] = loadBitmap("leopard.bmp");
	textures[2] = loadBitmap("giraffe.bmp");
	textures[3] = loadBitmap("cow.bmp");
	PNGProcessor pngprocess;
	textures[4] = pngprocess.createFurTextures(383832, 128, 20, furDensity, "furPattern.png");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
glm::vec3 moveForward(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d * std::sin(r*DEG_TO_RADIAN), pos.y, pos.z - d * std::cos(r*DEG_TO_RADIAN));
}

glm::vec3 moveRight(glm::vec3 pos, GLfloat angle, GLfloat d) {
	return glm::vec3(pos.x + d * std::cos(r*DEG_TO_RADIAN), pos.y, pos.z + d * std::sin(r*DEG_TO_RADIAN));
}

void update(void) {
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_UP]) { 
		layers += 1;
		cout << "Layers: " << layers << endl;
	}
	if (keys[SDL_SCANCODE_DOWN]) { 
		layers -= 1;
		if (layers < 1) layers = 1;
		cout << "Layers: " << layers << endl;
	}
	if (keys[SDL_SCANCODE_LEFT]) { 
		furLength -= 0.01;
		if (furLength < 0) furLength = 0;
		cout << "Fur Length: " << furLength << endl;
	}
	if (keys[SDL_SCANCODE_RIGHT]) { // Select Reflection Shader 
		furLength += 0.01;
		cout << "Fur Length: " << furLength << endl;
	}
	if (keys[SDL_SCANCODE_1]) {
		furPatternNum = 0;
		cout << "Fur Selected: Tiger" << endl;
	}
	if (keys[SDL_SCANCODE_2]) {
		furPatternNum = 1;
		cout << "Fur Selected: Leopard" << endl;
	}
	if (keys[SDL_SCANCODE_3]) {
		furPatternNum = 2;
		cout << "Fur Selected: Giraffe" << endl;
	}
	if (keys[SDL_SCANCODE_4]) {
		furPatternNum = 3;
		cout << "Fur Selected: Cow" << endl;
	}
	if (keys[SDL_SCANCODE_W]) eye = moveForward(eye, r, 0.1f);
	if (keys[SDL_SCANCODE_S]) eye = moveForward(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_A]) eye = moveRight(eye, r, -0.1f);
	if (keys[SDL_SCANCODE_D]) eye = moveRight(eye, r, 0.1f);
	if (keys[SDL_SCANCODE_R]) eye.y += 0.1;
	if (keys[SDL_SCANCODE_F]) eye.y -= 0.1;
	if (keys[SDL_SCANCODE_COMMA]) r -= 1.0f;
	if (keys[SDL_SCANCODE_PERIOD]) r += 1.0f;
}


void draw(SDL_Window * window) {
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up matrices
	glm::mat4 projection(1.0);
	projection = glm::perspective(float(60.0f*DEG_TO_RADIAN), 800.0f / 600.0f, 1.0f, 150.0f);
	GLfloat scale(1.0f);
	glm::mat4 modelview(1.0);
	mvStack.push(modelview);

	at = moveForward(eye, r, 1.0f);
	mvStack.top() = glm::lookAt(eye, at, up);

	// Draw Skybox
	glUseProgram(skyboxProgram);
	rt3d::setUniformMatrix4fv(skyboxProgram, "projection", glm::value_ptr(projection));

	glDepthMask(GL_FALSE); // make sure writing to update depth test is off
	glm::mat3 mvRotOnlyMat3 = glm::mat3(mvStack.top());
	mvStack.push(glm::mat4(mvRotOnlyMat3));
	glCullFace(GL_FRONT); // drawing inside of cube!

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox[0]);
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(2.0f, 2.0f, 2.0f));
	rt3d::setUniformMatrix4fv(skyboxProgram, "modelview", glm::value_ptr(mvStack.top()));
	rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);
	mvStack.pop();
	glCullFace(GL_BACK);

	// Enable depth test is on for the remainer of rendering.
	glDepthMask(GL_TRUE); 

	// Draw mesh with fur applied to it.
	glUseProgram(furProgram);
	rt3d::setUniformMatrix4fv(furProgram, "projection", glm::value_ptr(projection));

	mvStack.push(mvStack.top());
	mvStack.top() = glm::translate(mvStack.top(), glm::vec3(0.0f, 0.0f, -5.0f));
	mvStack.top() = glm::scale(mvStack.top(), glm::vec3(1.0f, 1.0f, 1.0f));
	rt3d::setUniformMatrix4fv(furProgram, "modelview", glm::value_ptr(mvStack.top()));
	// Pass through the total amount of layers
	GLuint uniformIndex = glGetUniformLocation(furProgram, "layers");
	glUniform1f(uniformIndex, (float)layers);
	// Pass through fur length 
	uniformIndex = glGetUniformLocation(furProgram, "furLength");
	glUniform1f(uniformIndex, furLength);
	float num = 1;

	// Assign textures
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textures[furPatternNum]);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	
	for (int i = 0; i < layers; i++) {
		// Pass through currentLayer
		uniformIndex = glGetUniformLocation(furProgram, "currentLayer");
		glUniform1f(uniformIndex, (float)i);

		// Determine the alpha and pass it through via UVScale
		uniformIndex = glGetUniformLocation(furProgram, "UVScale");
		num = num - (1 / (float)layers);
		if (num > 1) num = 1;
		if (num < 0) num = 0;
		glUniform1f(uniformIndex, num);

		// Passthrough fur movement.
		uniformIndex = glGetUniformLocation(furProgram, "furFlowOffset");
		if (furFlowOffset > 0.01) {
			increment = false;
		}
		else if (furFlowOffset < -0.01) {
			increment = true;
		}
		if(increment) furFlowOffset += 0.00001;
		else furFlowOffset -= 0.00001;
		glUniform1f(uniformIndex, furFlowOffset * ((float)i / (float)layers));

		rt3d::drawIndexedMesh(meshObjects[0], meshIndexCount, GL_TRIANGLES);

	}
	mvStack.pop();

	mvStack.pop(); // initial matrix
	glDepthMask(GL_TRUE);

	SDL_GL_SwapWindow(window); // swap buffers
}

// Program entry point - SDL manages the actual WinMain entry point for us
int main(int argc, char *argv[]) {
	SDL_Window * hWindow; // window handle
	SDL_GLContext glContext; // OpenGL context handle
	hWindow = setupRC(glContext); // Create window and render context 

	// Required on Windows *only* init GLEW to access OpenGL beyond 1.1
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (GLEW_OK != err) { // glewInit failed, something is seriously wrong
		std::cout << "glewInit failed, aborting." << endl;
		exit(1);
	}
	cout << glGetString(GL_VERSION) << endl;

	init();

	bool running = true; // set running to true
	SDL_Event sdlEvent;  // variable to detect SDL events
	while (running) {	// the event loop
		while (SDL_PollEvent(&sdlEvent)) {
			if (sdlEvent.type == SDL_QUIT)
				running = false;
		}
		update();
		draw(hWindow); // call the draw function
	}

	SDL_GL_DeleteContext(glContext);
	SDL_DestroyWindow(hWindow);
	SDL_Quit();
	return 0;
}