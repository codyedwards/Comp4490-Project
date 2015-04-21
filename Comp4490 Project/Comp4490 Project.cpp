#include <iostream>
#include <string>
#include <random>

#define GLEW_STATIC
#include <GL\glew.h>

#include <GLFW\glfw3.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\type_ptr.hpp>

#include "Shader.h"
#include "camera.h"
#include "Model.h"

#include <SOIL.h>

#define PI 3.14159265
#define NUM_WAVES 4
#define NUM_TEX_WAVES 15

using namespace std;
using namespace glm;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
GLuint loadTexture(GLchar* path);
GLuint loadCubemap(vector<const GLchar*> faces);

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Waves
GLfloat amplitudes[NUM_WAVES];
GLfloat waveLengths[NUM_WAVES];
GLfloat speeds[NUM_WAVES];
GLfloat directions[NUM_WAVES*2];

// Texture Waves
GLfloat texAmplitudes[NUM_TEX_WAVES];
GLfloat texWaveLengths[NUM_TEX_WAVES];
GLfloat texSpeeds[NUM_TEX_WAVES];
GLfloat texDirections[NUM_TEX_WAVES * 2];

// Camera
Camera camera(vec3 (0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Wave constraints
GLfloat mA = 0.1f;
GLfloat mL = 8.0f;
GLfloat mS = 1.0f;
vec2 mD = vec2(1.0f, 0.0f);
GLdouble theta = 2.0f;
GLfloat kAmpOverLen = mA / mL;
GLfloat Q = 0.85f;

GLfloat tex_mL = 3.0f;
GLfloat tex_kAmpOverLen = mA / tex_mL;

bool wiremesh = false;

GLfloat lightDirection[3] = { -0.1666666f, -1.0f, 0.35609757f };

void createWaves()
{
	default_random_engine generator;
	uniform_real_distribution<float> randL(mL/2.0f, mL * 2.0f);
	uniform_real_distribution<float> randTexL(tex_mL / 2.0f, tex_mL * 2.0f);
	uniform_real_distribution<float> randS(mS - 0.1f, mS + 0.1f);
	uniform_real_distribution<double> randAngle(-theta, theta);
	uniform_real_distribution<float> randDir(0.0f, 1.0f);

	for (int i = 0, j = 0; i < 4; i++, j+=2)
	{
		waveLengths[i] = randL(generator);
		amplitudes[i] = waveLengths[i] * kAmpOverLen;
		speeds[i] = randS(generator);
		directions[j] = (float)sin(randAngle(generator) ) * mD.x;
		directions[j + 1] = (float)sin(randAngle(generator) ) * mD.y;
	}

	for (int i = 0, j = 0; i < 15; i++, j += 2)
	{
		texWaveLengths[i] = randTexL(generator);
		texAmplitudes[i] = texWaveLengths[i] * tex_kAmpOverLen;
		texSpeeds[i] = randS(generator);
		texDirections[j] = (float)sin(randAngle(generator)) * mD.x;
		texDirections[j + 1] = (float)sin(randAngle(generator)) * mD.y;
	}
}

int main()
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Cody Edwards Comp4490 Project", nullptr, nullptr);
	glfwSetWindowPos(window, 1000, 100);
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		cout << "Failed to create GLFW window" << endl;
		glfwTerminate();
		return -1;
	}

	// Set required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		cout << "Failed to initialize GLEW" << endl;
		return -1;
	}

	glViewport(0, 0, screenWidth, screenHeight);

	glEnable(GL_DEPTH_TEST);

	Shader ourShader("wave.vert", "wave.frag");
	Shader skyboxShader("skybox.vert", "skybox.frag");

	Model ourModel("grid/grid.obj");

	GLfloat skyboxVertices[] = {
		// Positions          
		-1.0f, 1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, -1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, -1.0f, 1.0f,
		-1.0f, -1.0f, 1.0f,

		-1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, -1.0f,
		1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, -1.0f,
		1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f, 1.0f,
		1.0f, -1.0f, 1.0f
	};

	// Setup skybox VAO
	GLuint skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glBindVertexArray(0);

	// Cubemap (Skybox)
	vector<const GLchar*> faces;
	faces.push_back("envmap_miramar/miramar_rt.tga");
	faces.push_back("envmap_miramar/miramar_lf.tga");
	faces.push_back("envmap_miramar/miramar_up.tga");
	faces.push_back("envmap_miramar/miramar_dn.tga");
	faces.push_back("envmap_miramar/miramar_bk.tga");
	faces.push_back("envmap_miramar/miramar_ft.tga");
	GLuint cubemapTexture = loadCubemap(faces);

	createWaves();

	// Show controls
	cout << "           Space -> Change to wireframe mode and vice-versa" << endl;
	cout << "               1 -> Decrease amplitude" << endl;
	cout << "               2 -> Increase amplitude" << endl;
	cout << "               3 -> Decrease wavelength" << endl;
	cout << "               4 -> Increase wavelength" << endl;
	cout << "               5 -> Decrease x direction" << endl;
	cout << "               6 -> Increase x direction" << endl;
	cout << "               7 -> Decrease y direction" << endl;
	cout << "               8 -> Increase y direction" << endl;
	cout << "               9 -> Decrease speed" << endl;
	cout << "               0 -> Increase speed" << endl;
	cout << "      Left Arrow -> Move light left" << endl;
	cout << "     Right Arrow -> Move light right" << endl;
	cout << "      Down Arrow -> Move light up" << endl;
	cout << "        Up Arrow -> Move light down" << endl;
	cout << "  Shift Up Arrow -> Move light in" << endl;
	cout << "Shift Down Arrow -> Move light out" << endl;
	cout << "               - -> Decrease steepness of waves" << endl;
	cout << "               = -> Increase steepness of waves" << endl;
	cout << "   Use WASD and the mouse to move the camera" << endl;
	cout << "   Use scroll wheel to zoom in and out" << endl;

	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		Do_Movement();

		glClearColor(0.2f, 0.05f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (wiremesh)
			// Draw in wireframe
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Draw scene
		ourShader.use();

		// Time
		GLint time = glGetUniformLocation(ourShader.program, "time");;
		glUniform1f(time, currentFrame);

		GLint amps = glGetUniformLocation(ourShader.program, "A");
		GLint waveLen = glGetUniformLocation(ourShader.program, "L");
		GLint speed = glGetUniformLocation(ourShader.program, "speed");
		GLint dir = glGetUniformLocation(ourShader.program, "D");

		GLint texAmps = glGetUniformLocation(ourShader.program, "texA");
		GLint texWaveLen = glGetUniformLocation(ourShader.program, "texL");
		GLint texSpeed= glGetUniformLocation(ourShader.program, "texSpeed");
		GLint texDir = glGetUniformLocation(ourShader.program, "texDir");
		GLint steepness = glGetUniformLocation(ourShader.program, "Q");

		GLint ratio = glGetUniformLocation(ourShader.program, "kAmpOverLen");
		GLint texRatio = glGetUniformLocation(ourShader.program, "tex_kAmpOverLen");

		GLint lDir = glGetUniformLocation(ourShader.program, "LightPosition");

		glUniform1fv(amps, sizeof(amplitudes), amplitudes);
		glUniform1fv(waveLen, sizeof(waveLengths), waveLengths);
		glUniform1fv(speed, sizeof(speeds), speeds);
		glUniform2fv(dir, sizeof(directions), directions);

		glUniform1fv(texAmps, sizeof(texAmplitudes), texAmplitudes);
		glUniform1fv(texWaveLen, sizeof(texWaveLengths), texWaveLengths);
		glUniform1fv(texSpeed, sizeof(texSpeeds), texSpeeds);
		glUniform2fv(texDir, sizeof(texDirections), texDirections);
		glUniform1f(steepness, Q);

		glUniform1f(ratio, kAmpOverLen);
		glUniform1f(texRatio, tex_kAmpOverLen);

		glUniform3f(lDir, lightDirection[0], lightDirection[1], lightDirection[2]);

		// Transformation Matrices
		mat4 view = camera.GetViewMatrix();
		mat4 projection = perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		glUniformMatrix4fv(glGetUniformLocation(ourShader.program, "projection"), 1, GL_FALSE, value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(ourShader.program, "view"), 1, GL_FALSE, value_ptr(view));

		// Draw Model
		mat4 model;
		model = translate(model, vec3(0.0f, -1.75f, 0.0f));
		model = scale(model, vec3(0.25f, 0.25f, 0.25f));
		glUniformMatrix4fv(glGetUniformLocation(ourShader.program, "model"), 1, GL_FALSE, value_ptr(model));
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(ourShader.program, "skybox"), 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		ourModel.Draw(ourShader);

		if (wiremesh)
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Draw skybox
		glDepthFunc(GL_LEQUAL);
		skyboxShader.use();
		view = mat4(mat3(camera.GetViewMatrix()));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.program, "projection"), 1, GL_FALSE, value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(skyboxShader.program, "view"), 1, GL_FALSE, value_ptr(view));
		//skybox cube
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(skyboxShader.program, "skybox"), 0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}

// Loads a cubemap texture from 6 individual texture faces
// Order should be:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
GLuint loadCubemap(vector<const GLchar*> faces)
{
	GLuint textureID;
	glGenTextures(1, &textureID);

	int width, height;
	unsigned char* image;

	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	for (GLuint i = 0; i < faces.size(); i++)
	{
		int ch;

		image = SOIL_load_image(faces[i], &width, &height, &ch, SOIL_LOAD_RGB);
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return textureID;
}

#pragma region "User input"

// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (keys[GLFW_KEY_SPACE])
		wiremesh = !wiremesh;
	if (keys[GLFW_KEY_UP] && !keys[GLFW_KEY_RIGHT_SHIFT])
	{
		lightDirection[1] += 0.1f;
		//cout << lightDirection[0] << ", " << lightDirection[1] << ", " << lightDirection[2] << endl;
	}
	if (keys[GLFW_KEY_DOWN] && !keys[GLFW_KEY_RIGHT_SHIFT])
	{
		lightDirection[1] -= 0.1f;
		//cout << lightDirection[0] << ", " << lightDirection[1] << ", " << lightDirection[2] << endl;
	}
	if (keys[GLFW_KEY_LEFT])
	{
		lightDirection[0] -= 0.1f;
		//cout << lightDirection[0] << ", " << lightDirection[1] << ", " << lightDirection[2] << endl;
	}
	if (keys[GLFW_KEY_RIGHT])
	{
		lightDirection[0] += 0.1f;
		//cout << lightDirection[0] << ", " << lightDirection[1] << ", " << lightDirection[2] << endl;
	}
	if (keys[GLFW_KEY_UP] && keys[GLFW_KEY_RIGHT_SHIFT])
	{
		lightDirection[2] -= 0.1f;
		//cout << lightDirection[0] << ", " << lightDirection[1] << ", " << lightDirection[2] << endl;
	}
	if (keys[GLFW_KEY_DOWN] && keys[GLFW_KEY_RIGHT_SHIFT])
	{
		lightDirection[2] += 0.1f;
		//cout << lightDirection[0] << ", " << lightDirection[1] << ", " << lightDirection[2] << endl;
	}
	if (keys[GLFW_KEY_1] && mA > 0.1f)
	{
		mA -= 0.1f;
		kAmpOverLen = mA / mL;
		tex_kAmpOverLen = mA / tex_mL;
		createWaves();
	}
	if (keys[GLFW_KEY_2])
	{
		mA += 0.1f;
		kAmpOverLen = mA / mL;
		tex_kAmpOverLen = mA / tex_mL;
		createWaves();
	}
	if (keys[GLFW_KEY_3] && mL > 0.1f)
	{
		mL -= 0.1f;
		kAmpOverLen = mA / mL;
		createWaves();
	}
	if (keys[GLFW_KEY_4])
	{
		mL += 0.1f;
		kAmpOverLen = mA / mL;
		createWaves();
	}
	if (keys[GLFW_KEY_5] && mD[0] > -1.0f)
	{
		mD[0] -= 0.1f;
		createWaves();
	}
	if (keys[GLFW_KEY_6] && mD[0] < 1.0f)
	{
		mD[0] += 0.1f;
		createWaves();
	}
	if (keys[GLFW_KEY_7] && mD[1] > -1.0f)
	{
		mD[1] -= 0.1f;
		createWaves();
	}
	if (keys[GLFW_KEY_8] && mD[1] < 1.0f)
	{
		mD[1] += 0.1f;
		createWaves();
	}
	if (keys[GLFW_KEY_9] && mS > 0.0f)
	{
		mS -= 0.01f;
		createWaves();
	}
	if (keys[GLFW_KEY_0])
	{
		mS += 0.01f;
		createWaves();
	}
	if (keys[GLFW_KEY_MINUS] && Q > 0.0f)
		Q -= 0.01f;
	if (keys[GLFW_KEY_EQUAL] && Q < 1.0f)
		Q += 0.01f;
	
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

#pragma endregion