#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "stb_image.h"
#include "shaders.h"
#include "entity.h"
#include "obstacle.h"
#include "character.h"
#include "camera.h"

// VAOs for main game and ending scene
unsigned int vao;
unsigned int end_vao;

// Shader manager - loads shaders for main scene and for ending scene
ShaderManager shaderManager;

// Constant definitions for viewing frustrum
const float near_plane = 1.0f;
const float far_plane = 101.0f;

// Frame timing
float deltaTime = 0.0f; // time it took to process the last frame
float lastFrame = 0.0f; // timestamp of last frame

// Define area in which character is contained
const float runway_width = 10.0f;
const float runway_height = 20.0f;

// Constants for scoreboard pieces
float scoreboard_y = -0.3f;
float scoreboard_size = 0.1f;

// Obstacle info
Obstacle *obstacle_list;
const int dist_between_obstacles = 10;
const int obstacles_per_level = 5;
const int num_obstacles = (int) ((far_plane - near_plane)/dist_between_obstacles) + obstacles_per_level; // Number of obstacles to swap out each level
const float obstacle_width = 2.0f;

// The main character and associated info
Character character;
glm::vec3 characterStartPos = glm::vec3(0.0f, 0.0f, 20.0f);
float characterStartVelocity = -7.0f;
glm::vec3 distFromCharacterToCamera = glm::vec3(0.0, 1.7f, 6.0f);

// The camera object
Camera camera;

// Game logic
float distSinceLevelUp = -characterStartPos.z;
int score = 0;

// Variables to hold the digits of the player's final score
// Represent indices into texture array, so are affected by the number of other textures stored there for character, obstacles, and Game Over label (3)
// ex. if score = 1728, then score5 = 3, score4 = 4, score3 = 10, score2 = 5, score1 = 11
int score1 = 0;
int score2 = 0;
int score3 = 0;
int score4 = 0;
int score5 = 0;

// Window attributes
const char* windowTitle = "Bumble Run";
int windowWidth = 1700;
int windowHeight = 1000;

// Transformation matrix (transforms each object to right orientation, rotation/etc.)
glm::mat4 trans_mat = glm::mat4(1.0f);
// Model matrix (shifts from local coordinates to world coordinates, in relation to other objects)
glm::mat4 model = glm::mat4(1.0f);
// View matrix (the camera, shifts to user's point of view, i.e. move -5 along z axis to zoom out by 5)
glm::mat4 view = glm::mat4(1.0f);
// Projection matrix (clip space, determines which ones end up on screen)
glm::mat4 projection = glm::mat4(1.0f);
// Normal matrix (inverse of transpose of model matrix, prevents distortion due to scaling when calculating lighting)
glm::mat4 normal_mat = glm::mat4(1.0f);

// set up lighting position
glm::vec3 lightPos = glm::vec3(50.0f, 100.0f, 50.0f);


// deltaTime, runway_width, runway_height, character, camera, distFromCharacterToCamera
// Process any key signals that have come in since the last frame
void process_movement_controls(GLFWwindow* window) {
	// Movement is proportional to amount of time elapsed in last frame
	float moveby = 2.0f*deltaTime;
	
	// Set bounds on how far character can move in any direction, based on size of runway and character size
	float max_dist_x = (runway_width / 2.0f - 2.0f*character.scale.x);
	float max_dist_y = (runway_height / 2.0f - 2.0f*character.scale.y);

	// Check for left, right, up, or down keys and move camera accordingly
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) { // LEFT
		if (camera.position.x > -max_dist_x) {
			camera.position -= glm::normalize(glm::cross(camera.front, camera.up)) * moveby;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) { // RIGHT
		if (camera.position.x < max_dist_x) {
			camera.position += glm::normalize(glm::cross(camera.front, camera.up)) * moveby;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) { // UP
		if (camera.position.y < max_dist_y) {
			camera.position += moveby * camera.up;
		}
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) { // DOWN
		if (camera.position.y > -max_dist_y) {
			camera.position -= moveby * camera.up;
		}
	}
	
	// Update character position to match camera position and then change view matrix to reflect camera position
	character.position = camera.position - distFromCharacterToCamera;
	view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
}

// textures, index, filepath,
// Add a texture to the texture list (taken from LearnOpenGL tutorial)
void addTexture(unsigned int* textures, int index, const char *filepath) {
	glGenTextures(1, &textures[index]);
	glBindTexture(GL_TEXTURE_2D, textures[index]);
	// set the texture wrapping/filtering options (on the currently bound texture object)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load and generate the texture
	int width, height, nrChannels;
	unsigned char* image = stbi_load(filepath, &width, &height, &nrChannels, 0);
	if (image)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(image);
}

// Set up the VAO for obstacles and character
void initGameVAO() {
	glGenVertexArrays(1, &vao);

	// Set up VBO
	unsigned int vbo;
	glGenBuffers(1, &vbo);

	// Bind vao to OpenGL Vertex array
	glBindVertexArray(vao);
	// Bind vbo to OpenGL array buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Copy data from vertices array into vbo / array buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_verts), cube_verts, GL_STATIC_DRAW);

	// Link vertex attributes
	// Vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// Normals
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

// Set up the VAO for squares (Game Over label and scoreboard pieces)
void initEndScreenVAO() {
	glGenVertexArrays(1, &end_vao);

	// Set up VBO
	unsigned int vbo;
	glGenBuffers(1, &vbo);

	// Bind vao to OpenGL Vertex array
	glBindVertexArray(end_vao);
	// Bind vbo to OpenGL array buffer
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	// Copy data from vertices array into vbo / array buffer
	glBufferData(GL_ARRAY_BUFFER, sizeof(square_verts), square_verts, GL_STATIC_DRAW);

	// Link vertex attributes
	// Vertices
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Texture coordinates
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

// Update timing info for calculating velocities, etc.
void updateDeltaTime() {
	float currentFrame = glfwGetTime();
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
}

// Check if the character has collided with any of the obstacles
bool checkCollided() {
	for (int i = 0; i < num_obstacles; i++) {
		if (obstacle_list[i].collided(character)) {
			return true;
		}
	}
	return false;
}

// Move to the next level by increasing speed and score, and also by moving obstacles that have been moved past onto the end of the line
void levelUp() {	
	// Move obstacles from front to end of array, updating to random x positions as you go
	Obstacle temp[obstacles_per_level];
	for (int i = 0; i < obstacles_per_level; i++) {
		temp[i] = obstacle_list[i];
		float rand_val = (((float)std::rand() / (float)RAND_MAX) * runway_width) - (runway_width/2.0f);
		temp[i].position.x = rand_val;
		temp[i].position.z -= num_obstacles * dist_between_obstacles;
	}
	for (int i = 0; i < num_obstacles - obstacles_per_level; i++) {
		obstacle_list[i] = obstacle_list[i + obstacles_per_level];
	}
	for (int i = 0; i < obstacles_per_level; i++) {
		obstacle_list[num_obstacles - obstacles_per_level + i] = temp[i];
	}
	
	// Update velocity, level tracker and score
	character.velocity -= 1.0f;
	camera.velocity -= 1.0f;
	distSinceLevelUp = 0.0f;

	score++;
}

// Helper method, translates integer score into digits for scoreboard (only works for scores <= 99999)
void score_to_texture_indices() {
	int indices_in_use = 3;
	score1 = score % 10 + indices_in_use;
	score2 = (score / 10) % 10 + indices_in_use;
	score3 = (score / 100) % 10 + indices_in_use;
	score4 = (score / 1000) % 10 + indices_in_use;
	score5 = (score / 10000) % 10 + indices_in_use;
}

int main(void)
{
	// Set up GLFW, GLEW, and window
	GLFWwindow* window;
	if (!glfwInit())
		return -1;
	window = glfwCreateWindow(windowWidth, windowHeight, windowTitle, NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	glewInit();

	// Set up game shaders
	shaderManager = ShaderManager();
	shaderManager.initEntityShaders();


	// Generate textures
	unsigned int textures[13];
	addTexture(textures, 0, "Resources/tree.jpg");
	addTexture(textures, 1, "Resources/bee.jpg");
	addTexture(textures, 2, "Resources/game_over.jpg");
	addTexture(textures, 3, "Resources/0.jpg");
	addTexture(textures, 4, "Resources/1.jpg");
	addTexture(textures, 5, "Resources/2.jpg");
	addTexture(textures, 6, "Resources/3.jpg");
	addTexture(textures, 7, "Resources/4.jpg");
	addTexture(textures, 8, "Resources/5.jpg");
	addTexture(textures, 9, "Resources/6.jpg");
	addTexture(textures, 10, "Resources/7.jpg");
	addTexture(textures, 11, "Resources/8.jpg");
	addTexture(textures, 12, "Resources/9.jpg");


	// Set up obstacles
	Obstacle obstacles[num_obstacles];
	for (int i = 0; i < num_obstacles; i++) {
		float rand_val = (((float) std::rand() / (float) RAND_MAX) * runway_width) - (runway_width/2.0f);
		obstacles[i] = Obstacle(glm::vec3(rand_val,0.0f,i*-10.0f),glm::vec3(obstacle_width,runway_height,1.0f));
	}
	obstacle_list = obstacles;

	// Set up character
	character = Character(characterStartPos, characterStartVelocity,glm::vec3(0.5f,0.5f,0.5f));

	// Set up VAO for main game entities (character and obstacles)
	initGameVAO();

	// Set up camera and view/projection matrices
	camera = Camera(characterStartPos + distFromCharacterToCamera, characterStartVelocity);
	view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);
	projection = glm::perspective(glm::radians(45.0f), ((float) windowWidth) / ((float) windowHeight), near_plane, far_plane);

	
	// Start game loop -- continue until window is closed or character collides with an obstacle
	while (!glfwWindowShouldClose(window) && !checkCollided()) {
		// Handle game logic for this frame (frame timing, movement, leveling up)
		updateDeltaTime();
		process_movement_controls(window);
		if (distSinceLevelUp > obstacles_per_level*dist_between_obstacles) {
			levelUp();
		}

		// Set up transformation/model/normal matrices
		trans_mat = glm::mat4(1.0f);
		model = glm::mat4(1.0f);
		normal_mat = glm::transpose(glm::inverse(model));

		// Set clear color
		glClearColor(0.18f, 0.45f, 0.16f, 1.0f);

		// Clear back buffers, enable blending / depth testing
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);

		// Set uniform var locations
		int transformLocation = glGetUniformLocation(shaderManager.program, "transform_matrix");
		int modelLocation = glGetUniformLocation(shaderManager.program, "model_matrix");
		int viewLocation = glGetUniformLocation(shaderManager.program, "view_matrix");
		int projectionLocation = glGetUniformLocation(shaderManager.program, "projection_matrix");
		int normalLocation = glGetUniformLocation(shaderManager.program, "normal_matrix");
		int lightPosLocation = glGetUniformLocation(shaderManager.program, "light_pos");
		int viewerPosLocation = glGetUniformLocation(shaderManager.program, "viewer_pos");


		// Use shader program
		glUseProgram(shaderManager.program);

		// OBSTACLE DRAWING SECTION
		
		// Set uniform vars that are constant between obstacles
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(normalLocation, 1, GL_FALSE, glm::value_ptr(normal_mat));
		glUniform3f(lightPosLocation, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(viewerPosLocation, camera.position.x, camera.position.y, camera.position.z);


		// Bind obstacle texture
		glBindTexture(GL_TEXTURE_2D, textures[0]);
		
		// Draw obstacles
		for (int i=0; i<num_obstacles; i++) {
			// Set uniform vars that change between obstacles (transformation, model)
			trans_mat = glm::mat4(1.0f);
			trans_mat = glm::scale(trans_mat, obstacle_list[i].scale);
			model = glm::mat4(1.0f);
			model = glm::translate(model, obstacle_list[i].position);
			glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans_mat));
			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));

			// Draw this obstacle
			glBindVertexArray(vao);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		

		// CHARACTER DRAWING SECTION

		// Update character / camera position and levelup tracker
		character.position.z += character.velocity * deltaTime;
		camera.position.z += camera.velocity * deltaTime;
		distSinceLevelUp -= character.velocity * deltaTime;
		
		// Bind character texture
		glBindTexture(GL_TEXTURE_2D, textures[1]);

		// Set up matrices for character
		trans_mat = glm::mat4(1.0f);
		trans_mat = glm::scale(trans_mat, character.scale);

		model = glm::mat4(1.0f);
		model = glm::translate(model, character.position);
		normal_mat = glm::transpose(glm::inverse(model));
		
		view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);

		// Bind the uniform vars that have changed
		glUniformMatrix4fv(transformLocation, 1, GL_FALSE, glm::value_ptr(trans_mat));
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(normalLocation, 1, GL_FALSE, glm::value_ptr(normal_mat));

		// Draw the character
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Swap filled back buffer with outdated front buffer
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// END GAME DRAWING SECTION

	// Clear back buffer
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set up shader + VAO for squares / end game shapes
	shaderManager.initEndScreenShaders();
	initEndScreenVAO();

	// Bind texture for Game Over label
	glBindTexture(GL_TEXTURE_2D, textures[2]);

	// Set up uniform vars and bind shader program
	int translateLocation = glGetUniformLocation(shaderManager.program, "translate");
	int scaleLocation = glGetUniformLocation(shaderManager.program, "scale");

	glUseProgram(shaderManager.program);

	// Init Game Over Label uniform vars
	model = glm::mat4(1.0f);
	glUniform2f(translateLocation, 0.0f, 0.4f);
	glUniformMatrix4fv(scaleLocation, 1, GL_FALSE, glm::value_ptr(model));
	
	// Draw Game Over Label
	glDrawArrays(GL_POLYGON, 0, 4);


	// Draw scoreboard
	model = glm::scale(model, glm::vec3(scoreboard_size, scoreboard_size, 1.0f));
	score_to_texture_indices();

	glBindTexture(GL_TEXTURE_2D, textures[score5]);
	glUniform2f(translateLocation, -2.0f* scoreboard_size, scoreboard_y);
	glUniformMatrix4fv(scaleLocation, 1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_POLYGON, 0, 4);

	glBindTexture(GL_TEXTURE_2D, textures[score4]);
	glUniform2f(translateLocation, -scoreboard_size, scoreboard_y);
	glUniformMatrix4fv(scaleLocation, 1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_POLYGON, 0, 4);

	glBindTexture(GL_TEXTURE_2D, textures[score3]);
	glUniform2f(translateLocation, 0.0f, scoreboard_y);
	glUniformMatrix4fv(scaleLocation, 1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_POLYGON, 0, 4);

	glBindTexture(GL_TEXTURE_2D, textures[score2]);
	glUniform2f(translateLocation, scoreboard_size, scoreboard_y);
	glUniformMatrix4fv(scaleLocation, 1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_POLYGON, 0, 4);

	glBindTexture(GL_TEXTURE_2D, textures[score1]);
	glUniform2f(translateLocation, 2.0f * scoreboard_size, scoreboard_y);
	glUniformMatrix4fv(scaleLocation, 1, GL_FALSE, glm::value_ptr(model));
	glDrawArrays(GL_POLYGON, 0, 4);

	glfwSwapBuffers(window);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	// Clean up and return
	glfwTerminate();
	return 0;
}