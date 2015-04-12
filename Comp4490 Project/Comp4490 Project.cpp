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
#include "Wave.h"

#include "SOIL.h"

#define PI 3.14159265
#define NUM_WAVES 4

using namespace std;
using namespace glm;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();

// Properties
GLuint screenWidth = 800, screenHeight = 600;

// Waves
GLfloat amplitudes[NUM_WAVES];
GLfloat waveLengths[NUM_WAVES];
GLfloat speeds[NUM_WAVES];
GLfloat directions[NUM_WAVES*2];

// Camera
Camera camera(vec3 (0.0f, 0.0f, 3.0f));
bool keys[1024];
GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

GLfloat mA = 0.15f;
GLfloat mL = 2.5f;
GLfloat mS = 0.5f;
//vec2 mD = vec2(1.0f, 1.0f);
GLdouble theta = 4.0f;

void createWaves()
{
	default_random_engine generator;
	uniform_real_distribution<float> randL(mL/2.0f, mL * 2.0f);
	uniform_real_distribution<float> randS(mS / 2.0f, mS * 2.0f);
	uniform_real_distribution<double> randAngle(-theta, theta);
	uniform_real_distribution<float> randDir(0.0f, 1.0f);
	vec2 mD = vec2(randDir(generator), randDir(generator));

	for (int i = 0, j = 0; i < 4; i++, j+=2)
	{
		waveLengths[i] = randL(generator);
		amplitudes[i] = (waveLengths[i] / mL) * mA;
		speeds[i] = randS(generator);
		directions[j] = (float)sin(randAngle(generator) ) * mD.x;
		directions[j + 1] = (float)sin(randAngle(generator) ) * mD.y;
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

	Shader ourShader("default.vert", "default.frag");

	Model ourModel("grid.obj");

	// Draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	createWaves();

	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		glfwPollEvents();
		Do_Movement();

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.use();

		// Time
		GLint time = glGetUniformLocation(ourShader.program, "time");;
		glUniform1f(time, currentFrame);

		GLint amps = glGetUniformLocation(ourShader.program, "A");
		GLint waveLen = glGetUniformLocation(ourShader.program, "L");
		GLint speed = glGetUniformLocation(ourShader.program, "speed");
		GLint dir = glGetUniformLocation(ourShader.program, "D");

		glUniform1fv(amps, sizeof(amplitudes), amplitudes);
		glUniform1fv(waveLen, sizeof(waveLengths), waveLengths);
		glUniform1fv(speed, sizeof(speeds), speeds);
		glUniform2fv(dir, sizeof(directions), directions);

		// Transformation Matrices
		mat4 projection = perspective(camera.Zoom, (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
		mat4 view = camera.GetViewMatrix();
		glUniformMatrix4fv(glGetUniformLocation(ourShader.program, "projection"), 1, GL_FALSE, value_ptr(projection));
		glUniformMatrix4fv(glGetUniformLocation(ourShader.program, "view"), 1, GL_FALSE, value_ptr(view));

		// Draw Model
		mat4 model;
		model = translate(model, vec3(0.0f, -1.75f, 0.0f));
		model = scale(model, vec3(0.2f, 0.2f, 0.2f));
		glUniformMatrix4fv(glGetUniformLocation(ourShader.program, "model"), 1, GL_FALSE, value_ptr(model));
		ourModel.Draw(ourShader);

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
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