#include "Demo.h"



Demo::Demo() {

}


Demo::~Demo() {
}



void Demo::Init() {
	BuildShaders();
	BuildDepthMap();
	BuildTexturedCube();
	BuildTexturedPlane();
	InitCamera();
}

void Demo::DeInit() {
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);
	glDeleteBuffers(1, &planeEBO);
	glDeleteBuffers(1, &depthMapFBO);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Demo::ProcessInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	// zoom camera
	// -----------
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		if (fovy < 90) {
			fovy += 0.0001f;
		}
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (fovy > 0) {
			fovy -= 0.0001f;
		}
	}

	// update camera movement 
	// -------------
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		MoveCamera(CAMERA_SPEED);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		MoveCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		StrafeCamera(-CAMERA_SPEED);
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		StrafeCamera(CAMERA_SPEED);
	}

	// update camera rotation
	// ----------------------
	double mouseX, mouseY;
	double midX = screenWidth / 2;
	double midY = screenHeight / 2;
	float angleY = 0.0f;
	float angleZ = 0.0f;

	// Get mouse position
	glfwGetCursorPos(window, &mouseX, &mouseY);
	if ((mouseX == midX) && (mouseY == midY)) {
		return;
	}

	// Set mouse position
	glfwSetCursorPos(window, midX, midY);

	// Get the direction from the mouse cursor, set a resonable maneuvering speed
	angleY = (float)((midX - mouseX)) / 1000;
	angleZ = (float)((midY - mouseY)) / 1000;

	// The higher the value is the faster the camera looks around.
	viewCamY += angleZ * 2;

	// limit the rotation around the x-axis
	if ((viewCamY - posCamY) > 8) {
		viewCamY = posCamY + 8;
	}
	if ((viewCamY - posCamY) < -8) {
		viewCamY = posCamY - 8;
	}
	RotateCamera(-angleY);
}

void Demo::Update(double deltaTime) {
	angle += (float)((deltaTime * 1.5f) / 1000);
}

void Demo::Render() {
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_ALPHA_TEST);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	
	// Step 1 Render depth of scene to texture
	// ----------------------------------------
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	UseShader(this->depthmapShader);
	glUniformMatrix4fv(glGetUniformLocation(this->depthmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glViewport(0, 0, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawTexturedCube(this->depthmapShader);
	DrawTexturedPlane(this->depthmapShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	

	// Step 2 Render scene normally using generated depth map
	// ------------------------------------------------------
	glViewport(0, 0, this->screenWidth, this->screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Pass perspective projection matrix
	UseShader(this->shadowmapShader);
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 100.0f);
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	// LookAt camera (position, target/direction, up)
	glm::vec3 cameraPos = glm::vec3(posCamX, posCamY, posCamZ);
	glm::vec3 cameraFront = glm::vec3(viewCamX, viewCamY, viewCamZ);
	glm::mat4 view = glm::lookAt(cameraPos, cameraFront, glm::vec3(upCamX, upCamY, upCamZ));
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "view"), 1, GL_FALSE, glm::value_ptr(view));
	
	// Setting Light Attributes
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "lightPos"), -2.0f, 4.0f, -1.0f);

	// Configure Shaders
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "shadowMap"), 1);

	

	// Render floor
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedPlane(this->shadowmapShader);
	
	// Render cube
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawTexturedCube(this->shadowmapShader);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
}



void Demo::BuildTexturedCube()
{
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 
	glGenTextures(1, &cube_texture);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height;
	unsigned char* image = SOIL_load_image("crate.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);


	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// format position, tex coords

		// Alas Meja
		// front
		-0.5,  0.3,  0.5, 0, 0, 0.0f,  0.0f,  1.0f,// 0
		 0.5,  0.3,  0.5, 1, 0, 0.0f,  0.0f,  1.0f,// 1
		 0.5,  0.5,  0.5, 1, 1, 0.0f,  0.0f,  1.0f,// 2
		-0.5,  0.5,  0.5, 0, 1, 0.0f,  0.0f,  1.0f,// 3

		// right
		 0.5,  0.5,  0.5, 0, 0, 1.0f,  0.0f,  0.0f,// 4
		 0.5,  0.5, -0.5, 1, 0, 1.0f,  0.0f,  0.0f,// 5
		 0.5,  0.3, -0.5, 1, 1, 1.0f,  0.0f,  0.0f,// 6
		 0.5,  0.3,  0.5, 0, 1, 1.0f,  0.0f,  0.0f,// 7

		// back
		-0.5,  0.3, -0.5, 0, 0, 0.0f,  0.0f,  -1.0f,// 8
		 0.5,  0.3, -0.5, 1, 0, 0.0f,  0.0f,  -1.0f,// 9
		 0.5,  0.5, -0.5, 1, 1, 0.0f,  0.0f,  -1.0f,// 10
		-0.5,  0.5, -0.5, 0, 1, 0.0f,  0.0f,  -1.0f,// 11

		// left
		-0.5,  0.3, -0.5, 0, 0, -1.0f,  0.0f,  0.0f,// 12
		-0.5,  0.3,  0.5, 1, 0, -1.0f,  0.0f,  0.0f,// 13
		-0.5,  0.5,  0.5, 1, 1, -1.0f,  0.0f,  0.0f,// 14
		-0.5,  0.5, -0.5, 0, 1, -1.0f,  0.0f,  0.0f,// 15

		// upper
		 0.5,  0.5,  0.5, 0, 0, 0.0f,  1.0f,  0.0f,// 16
		-0.5,  0.5,  0.5, 1, 0, 0.0f,  1.0f,  0.0f,// 17
		-0.5,  0.5, -0.5, 1, 1, 0.0f,  1.0f,  0.0f,// 18
		 0.5,  0.5, -0.5, 0, 1, 0.0f,  1.0f,  0.0f,// 19

		// bottom
		-0.5,  0.3, -0.5, 0, 0, 0.0f,  -1.0f,  0.0f,// 20
		 0.5,  0.3, -0.5, 1, 0, 0.0f,  -1.0f,  0.0f,// 21
		 0.5,  0.3,  0.5, 1, 1, 0.0f,  -1.0f,  0.0f,// 22
		-0.5,  0.3,  0.5, 0, 1, 0.0f,  -1.0f,  0.0f,// 23

		// Kaki kiri depan
		// front
		-0.5, -0.5,  0.5, 0, 0, 0.0f,  0.0f,  1.0f,// 24
		-0.4, -0.5,  0.5, 1, 0, 0.0f,  0.0f,  1.0f,// 25
		-0.4,  0.4,  0.5, 1, 1, 0.0f,  0.0f,  1.0f,// 26
		-0.5,  0.4,  0.5, 0, 1, 0.0f,  0.0f,  1.0f,// 27

		// right
		-0.4,  0.4,  0.5, 0, 0, 1.0f,  0.0f,  0.0f,// 28
		-0.4,  0.4,  0.4, 1, 0, 1.0f,  0.0f,  0.0f,// 29
		-0.4, -0.5,  0.4, 1, 1, 1.0f,  0.0f,  0.0f,// 30
		-0.4, -0.5,  0.5, 0, 1, 1.0f,  0.0f,  0.0f,// 31

		// back
		-0.5, -0.5,  0.4, 0, 0, 0.0f,  0.0f,  -1.0f,// 32
		-0.4, -0.5,  0.4, 1, 0, 0.0f,  0.0f,  -1.0f,// 33
		-0.4,  0.4,  0.4, 1, 1, 0.0f,  0.0f,  -1.0f,// 34
		-0.5,  0.4,  0.4, 0, 1, 0.0f,  0.0f,  -1.0f,// 35

		// left
		-0.5, -0.5,  0.4, 0, 0, -1.0f,  0.0f,  0.0f,// 36
		-0.5, -0.5,  0.5, 1, 0, -1.0f,  0.0f,  0.0f,// 37
		-0.5,  0.4,  0.5, 1, 1, -1.0f,  0.0f,  0.0f,// 38
		-0.5,  0.4,  0.4, 0, 1, -1.0f,  0.0f,  0.0f,// 39

		// upper
		-0.4,  0.4,  0.5, 0, 0, 0.0f,  1.0f,  0.0f,// 40
		-0.5,  0.4,  0.5, 1, 0, 0.0f,  1.0f,  0.0f,// 41
		-0.5,  0.4,  0.4, 1, 1, 0.0f,  1.0f,  0.0f,// 42
		-0.4,  0.4,  0.4, 0, 1, 0.0f,  1.0f,  0.0f,// 43

		// bottom
		-0.5, -0.5,  0.4, 0, 0, 0.0f, -1.0f, 0.0f,// 44
		-0.4, -0.5,  0.4, 1, 0, 0.0f, -1.0f, 0.0f,// 45
		-0.4, -0.5,  0.5, 1, 1, 0.0f, -1.0f, 0.0f,// 46
		-0.5, -0.5,  0.5, 0, 1, 0.0f, -1.0f, 0.0f,// 47

		// Kaki kanan depan
		// front
		 0.4, -0.5,  0.5, 0, 0, 0.0f,  0.0f,  1.0f,//48
		 0.5, -0.5,  0.5, 1, 0, 0.0f,  0.0f,  1.0f,//49
		 0.5,  0.4,  0.5, 1, 1, 0.0f,  0.0f,  1.0f,//50
		 0.4,  0.4,  0.5, 0, 1, 0.0f,  0.0f,  1.0f,//51

		// right
		 0.5,  0.4,  0.5, 0, 0, 1.0f,  0.0f,  0.0f,//52
		 0.5,  0.4,  0.4, 1, 0, 1.0f,  0.0f,  0.0f,//53
		 0.5, -0.5,  0.4, 1, 1, 1.0f,  0.0f,  0.0f,//54
		 0.5, -0.5,  0.5, 0, 1, 1.0f,  0.0f,  0.0f,//55

		// back
		 0.4, -0.5,  0.4, 0, 0, 0.0f, 0.0f, -1.0f,//56
		 0.5, -0.5,  0.4, 1, 0, 0.0f, 0.0f, -1.0f,//57
		 0.5,  0.4,  0.4, 1, 1, 0.0f, 0.0f, -1.0f,//58
		 0.4,  0.4,  0.4, 0, 1, 0.0f, 0.0f, -1.0f,//59

		// left
		 0.4, -0.5,  0.4, 0, 0, -1.0f, 0.0f, 0.0f,//60
		 0.4, -0.5,  0.5, 1, 0, -1.0f, 0.0f, 0.0f,//61
		 0.4,  0.4,  0.5, 1, 1, -1.0f, 0.0f, 0.0f,//62
		 0.4,  0.4,  0.4, 0, 1, -1.0f, 0.0f, 0.0f,//63

		// upper
		 0.5,  0.4,  0.5, 0, 0, 0.0f, 1.0f, 0.0f,//64
		 0.4,  0.4,  0.5, 1, 0, 0.0f, 1.0f, 0.0f,//65
		 0.5,  0.4,  0.4, 1, 1, 0.0f, 1.0f, 0.0f,//66
		 0.5,  0.4,  0.4, 0, 1, 0.0f, 1.0f, 0.0f,//67

		// bottom
		 0.4, -0.5,  0.4, 0, 0, 0.0f, -1.0f, 0.0f,//68
		 0.5, -0.5,  0.4, 1, 0, 0.0f, -1.0f, 0.0f,//69
		 0.5, -0.5,  0.5, 1, 1, 0.0f, -1.0f, 0.0f,//70
		 0.4, -0.5,  0.5, 0, 1, 0.0f, -1.0f, 0.0f,//71

		// Kaki kanan belakang
		// front
		 0.4, -0.5, -0.4, 0, 0, 0.0f, 0.0f, 1.0f,//72
		 0.5, -0.5, -0.4, 1, 0, 0.0f, 0.0f, 1.0f,//73
		 0.5,  0.4, -0.4, 1, 1, 0.0f, 0.0f, 1.0f,//74
		 0.4,  0.4, -0.4, 0, 1, 0.0f, 0.0f, 1.0f,//75

		// right
		 0.5,  0.4, -0.4, 0, 0, 1.0f, 0.0f, 0.0f,//76
		 0.5,  0.4, -0.5, 1, 0, 1.0f, 0.0f, 0.0f,//77
		 0.5, -0.5, -0.5, 1, 1, 1.0f, 0.0f, 0.0f,//78
		 0.5, -0.5, -0.4, 0, 1, 1.0f, 0.0f, 0.0f,//79

		// back
		 0.4, -0.5, -0.5, 0, 0, 0.0f, 0.0f, -1.0f,//80
		 0.5, -0.5, -0.5, 1, 0, 0.0f, 0.0f, -1.0f,//81
		 0.5,  0.4, -0.5, 1, 1, 0.0f, 0.0f, -1.0f,//82
		 0.4,  0.4, -0.5, 0, 1, 0.0f, 0.0f, -1.0f,//83

		// left
		 0.4, -0.5, -0.5, 0, 0, -1.0f, 0.0f, 0.0f,//84
		 0.4, -0.5, -0.4, 1, 0, -1.0f, 0.0f, 0.0f,//85
		 0.4,  0.4, -0.4, 1, 1, -1.0f, 0.0f, 0.0f,//86
		 0.4,  0.4, -0.5, 0, 1, -1.0f, 0.0f, 0.0f,//87

		// upper
		 0.5,  0.4, -0.4, 0, 0, 0.0f, 1.0f, 0.0f,//88
		 0.4,  0.4, -0.4, 1, 0, 0.0f, 1.0f, 0.0f,//89
		 0.5,  0.4, -0.5, 1, 1, 0.0f, 1.0f, 0.0f,//90
		 0.5,  0.4, -0.5, 0, 1, 0.0f, 1.0f, 0.0f,//91

		// bottom
		 0.4, -0.5, -0.5, 0, 0, 0.0f, -1.0f, 0.0f,//92
		 0.5, -0.5, -0.5, 1, 0, 0.0f, -1.0f, 0.0f,//93
		 0.5, -0.5, -0.4, 1, 1, 0.0f, -1.0f, 0.0f,//94
		 0.4, -0.5, -0.4, 0, 1, 0.0f, -1.0f, 0.0f,//95

		// Kaki kiri belakang
		// front
		-0.5, -0.5, -0.4, 0, 0, 0.0f, 0.0f, 1.0f,//96
		-0.4, -0.5, -0.4, 1, 0, 0.0f, 0.0f, 1.0f,//97
		-0.4,  0.4, -0.4, 1, 1, 0.0f, 0.0f, 1.0f,//98
		-0.5,  0.4, -0.4, 0, 1, 0.0f, 0.0f, 1.0f,//99

		// right
		-0.4,  0.4, -0.4, 0, 0, 1.0f, 0.0f, 0.0f,//100
		-0.4,  0.4, -0.5, 1, 0, 1.0f, 0.0f, 0.0f,//101
		-0.4, -0.5, -0.5, 1, 1, 1.0f, 0.0f, 0.0f,//102
		-0.4, -0.5, -0.4, 0, 1, 1.0f, 0.0f, 0.0f,//103

		// back
		-0.5, -0.5, -0.5, 0, 0, 0.0f, 0.0f, -1.0f,//104
		-0.4, -0.5, -0.5, 1, 0, 0.0f, 0.0f, -1.0f,//105
		-0.4,  0.4, -0.5, 1, 1, 0.0f, 0.0f, -1.0f,//106
		-0.5,  0.4, -0.5, 0, 1, 0.0f, 0.0f, -1.0f,//107

		// left
		-0.5, -0.5, -0.5, 0, 0, -1.0f, 0.0f, 0.0f,//108
		-0.5, -0.5, -0.4, 1, 0, -1.0f, 0.0f, 0.0f,//109
		-0.5,  0.4, -0.4, 1, 1, -1.0f, 0.0f, 0.0f,//110
		-0.5,  0.4, -0.5, 0, 1, -1.0f, 0.0f, 0.0f,//111

		// upper
		-0.4,  0.4, -0.4, 0, 0, 0.0f, 1.0f, 0.0f,//112
		-0.5,  0.4, -0.4, 1, 0, 0.0f, 1.0f, 0.0f,//113
		-0.5,  0.4, -0.5, 1, 1, 0.0f, 1.0f, 0.0f,//114
		-0.4,  0.4, -0.5, 0, 1, 0.0f, 1.0f, 0.0f,//115

		// bottom
		-0.5, -0.5, -0.5, 0, 0, 0.0f, -1.0f, 0.0f,//116
		-0.4, -0.5, -0.5, 1, 0, 0.0f, -1.0f, 0.0f,//117
		-0.4, -0.5, -0.4, 1, 1, 0.0f, -1.0f, 0.0f,//118
		-0.5, -0.5, -0.4, 0, 1, 0.0f, -1.0f, 0.0f,//119

	};

	unsigned int indices[] = {
		// Alas Meja
		0,  1,  2,  0,  2,  3,   // front
		4,  5,  6,  4,  6,  7,   // right
		8,  9,  10, 8,  10, 11,  // back
		12, 14, 13, 12, 15, 14,  // left
		16, 18, 17, 16, 19, 18,  // upper
		20, 22, 21, 20, 23, 22,  // bottom

		// Kaki kiri depan
		24,25,26,24,26,27, // front
		28,29,30,28,30,31, // right
		32,33,34,32,34,35,  // back
		36,38,37,36,39,38,  // left
		40,42,41,40,43,42,  // upper
		44,46,45,44,47,46,  // bottom

		// Kaki kanan depan
		48,49,50,48,50,51, // front
		52,53,54,52,54,55, // right
		56,57,58,56,58,59, // back
		60,62,61,60,63,62, // left
		64,66,65,64,67,66, // upper
		68,70,69,68,71,70, // bottom

		// Kaki kanan belakang
		72,73,74,72,74,75, // front
		76,77,78,76,78,79, // right
		80,81,82,80,82,83, // back
		84,86,85,84,87,86, // left
		88,90,89,88,91,90, // upper
		92,94,93,92,95,94, // bottom

		// Kaki kiri belakang
		96,97,98,96,98,99,  // front
		100,101,102,100,102,103, // right
		104,105,106,104,106,107, // back
		108,110,109,108,111,110, // left
		112,114,113,112,115,114, // upper
		116,118,117,116,119,118 // bottom
	};

	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &cubeVBO);
	glGenBuffers(1, &cubeEBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(cubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Demo::BuildTexturedPlane()
{
	// Load and create a texture 
	glGenTextures(1, &plane_texture);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* image = SOIL_load_image("wood.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);
	

	// Build geometry
	GLfloat vertices[] = {
		// format position, tex coords
		// bottom
		-25.0f,	-0.5f, -25.0f,  0,  0, 0.0f,  1.0f,  0.0f,
		25.0f,	-0.5f, -25.0f, 25,  0, 0.0f,  1.0f,  0.0f,
		25.0f,	-0.5f,  25.0f, 25, 25, 0.0f,  1.0f,  0.0f,
		-25.0f,	-0.5f,  25.0f,  0, 25, 0.0f,  1.0f,  0.0f,
	};

	GLuint indices[] = { 0,  2,  1,  0,  3,  2 };

	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glGenBuffers(1, &planeEBO);

	glBindVertexArray(planeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	// Normal attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO
}

void Demo::DrawTexturedCube(GLuint shader)
{
	UseShader(shader);
	glBindVertexArray(cubeVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "material.diffuse"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, stexture);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "material.specular"), 1);

	GLint shininessMatLoc = glGetUniformLocation(this->shadowmapShader, "material.shininess");
	glUniform1f(shininessMatLoc, 0.4f);
	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 2, 0));

	model = glm::rotate(model, angle, glm::vec3(0, -1, 1));

	model = glm::scale(model, glm::vec3(2, 2, 2));

	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedPlane(GLuint shader)
{
	UseShader(shader);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, plane_texture);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "material.diffuse"), 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, stexture2);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "material.specular"), 3);

	GLint shininessMatLoc = glGetUniformLocation(this->shadowmapShader, "material.shininess");
	glUniform1f(shininessMatLoc, 0.4f);
	glBindVertexArray(planeVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glm::mat4 model;
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}



void Demo::BuildDepthMap() {
	// configure depth map FBO
	// -----------------------
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->SHADOW_WIDTH, this->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Demo::BuildShaders()
{
	// build and compile our shader program
	// ------------------------------------
	shadowmapShader = BuildShader("shadowMapping.vert", "shadowMapping.frag", nullptr);
	depthmapShader = BuildShader("depthMap.vert", "depthMap.frag", nullptr);
}
void Demo::InitCamera()
{
	posCamX = 0.0f;
	posCamY = 5.0f;
	posCamZ = 8.0f;
	viewCamX = 2.0f;
	viewCamY = 1.0f;
	viewCamZ = 0.0f;
	upCamX = 0.0f;
	upCamY = 1.0f;
	upCamZ = 0.0f;
	CAMERA_SPEED = 0.01f;
	fovy = 45.0f;
	glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


void Demo::MoveCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	// forward positive cameraspeed and backward negative -cameraspeed.
	posCamX = posCamX + x * speed;
	posCamZ = posCamZ + z * speed;
	viewCamX = viewCamX + x * speed;
	viewCamZ = viewCamZ + z * speed;
}

void Demo::StrafeCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	float orthoX = -z;
	float orthoZ = x;

	// left positive cameraspeed and right negative -cameraspeed.
	posCamX = posCamX + orthoX * speed;
	posCamZ = posCamZ + orthoZ * speed;
	viewCamX = viewCamX + orthoX * speed;
	viewCamZ = viewCamZ + orthoZ * speed;
}

void Demo::RotateCamera(float speed)
{
	float x = viewCamX - posCamX;
	float z = viewCamZ - posCamZ;
	viewCamZ = (float)(posCamZ + glm::sin(speed) * x + glm::cos(speed) * z);
	viewCamX = (float)(posCamX + glm::cos(speed) * x - glm::sin(speed) * z);
}

int main(int argc, char** argv) {
	RenderEngine &app = Demo();
	app.Start("Shadow Mapping Demo", 800, 600, false, false);
}