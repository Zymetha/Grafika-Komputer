#include "Demo.h"



Demo::Demo() {

}


Demo::~Demo() {
}



void Demo::Init() {
	// build and compile our shader program
	// ------------------------------------
	shadowmapShader = BuildShader("multipleLight.vert", "multipleLight.frag", nullptr);
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
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Demo::ProcessInput(GLFWwindow* window) {
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
	glViewport(0, 0, this->screenWidth, this->screenHeight);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_DEPTH_TEST);

	// Pass perspective projection matrix
	glm::mat4 projection = glm::perspective(fovy, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 100.0f);
	GLint projLoc = glGetUniformLocation(this->shadowmapShader, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// LookAt camera (position, target/direction, up)
	glm::vec3 cameraPos = glm::vec3(posCamX, posCamY, posCamZ);
	glm::vec3 cameraFront = glm::vec3(viewCamX, viewCamY, viewCamZ);
	glm::mat4 view = glm::lookAt(cameraPos, cameraFront, glm::vec3(upCamX, upCamY, upCamZ));
	GLint viewLoc = glGetUniformLocation(this->shadowmapShader, "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	// set lighting attributes
	GLint viewPosLoc = glGetUniformLocation(this->shadowmapShader, "viewPos");
	glUniform3f(viewPosLoc, cameraPos.x, cameraPos.y, cameraPos.z);

	glUniform3f(glGetUniformLocation(this->shadowmapShader, "dirLight.direction"), 0.0f, -1.0f, -1.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "dirLight.ambient"), 0.1f, 0.1f, 0.1f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "dirLight.diffuse"), 1.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "dirLight.specular"), 0.1f, 0.1f, 0.1f);
	// point light 1
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[0].position"), 0.0f, 3.0f, 0.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[0].ambient"), 1.0f, 0.0f, 1.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[0].diffuse"), 1.0f, 0.0f, 0.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[0].specular"), 1.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[0].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[0].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[0].quadratic"), 0.032f);
	// point light 2
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[1].position"), -2.0f, 3.0f, 0.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[1].ambient"), 0.0f, 1.0f, 0.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[1].diffuse"), 0.0f, 1.0f, 0.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[1].specular"), 0.0f, 1.0f, 0.0f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[1].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[1].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[1].quadratic"), 0.032f);
	// point light 3
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[2].position"), 2.0f, 3.0f, 0.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[2].ambient"), 0.0f, 0.0f, 1.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[2].diffuse"), 0.0f, 0.0f, 1.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[2].specular"), 0.0f, 0.0f, 1.0f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[2].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[2].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[2].quadratic"), 0.032f);
	// point light 4
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[3].position"), 0.0f, 3.0f, 2.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[3].ambient"), 0.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[3].diffuse"), 0.0f, 1.0f, 1.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "pointLights[3].specular"), 0.0f, 1.0f, 1.0f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[3].constant"), 1.0f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[3].linear"), 0.09f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "pointLights[3].quadratic"), 0.032f);
	// spotLight
	glUniform3fv(glGetUniformLocation(this->shadowmapShader, "spotLight.position"), 1, &cameraPos[0]);
	glUniform3fv(glGetUniformLocation(this->shadowmapShader, "spotLight.direction"), 1, &cameraFront[0]);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "spotLight.ambient"), 1.0f, 0.0f, 1.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "spotLight.diffuse"), 1.0f, 0.0f, 1.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "spotLight.specular"), 1.0f, 0.0f, 1.0f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "spotLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "spotLight.linear"), 0.09f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "spotLight.quadratic"), 0.032f);
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "spotLight.cutOff"), glm::cos(glm::radians(12.5f)));
	glUniform1f(glGetUniformLocation(this->shadowmapShader, "spotLight.outerCutOff"), glm::cos(glm::radians(15.0f)));

	DrawTexturedCube();
	DrawTexturedPlane();

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
	unsigned char* image = SOIL_load_image("crate_diffusemap.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &stexture);
	glBindTexture(GL_TEXTURE_2D, stexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	image = SOIL_load_image("crate_specularmap.png", &width, &height, 0, SOIL_LOAD_RGBA);
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
	unsigned char* image = SOIL_load_image("marble_diffusemap.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &stexture2);
	glBindTexture(GL_TEXTURE_2D, stexture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	image = SOIL_load_image("marble_specularmap.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Build geometry
	GLfloat vertices[] = {
		// format position, tex coords
		// bottom
		-50.0, -0.5, -50.0,  0,  0, 0.0f,  1.0f,  0.0f,
		50.0, -0.5, -50.0, 50,  0, 0.0f,  1.0f,  0.0f,
		50.0, -0.5,  50.0, 50, 50, 0.0f,  1.0f,  0.0f,
		-50.0, -0.5,  50.0,  0, 50, 0.0f,  1.0f,  0.0f,


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

void Demo::DrawTexturedCube()
{
	UseShader(this->shadowmapShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, cube_texture);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "material.diffuse"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, stexture);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "material.specular"), 1);

	GLint shininessMatLoc = glGetUniformLocation(this->shadowmapShader, "material.shininess");
	glUniform1f(shininessMatLoc, 0.4f);

	glBindVertexArray(cubeVAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glm::mat4 model;
	model = glm::translate(model, glm::vec3(0, 2, 0));

	model = glm::rotate(model, angle, glm::vec3(0, -1, 1));

	model = glm::scale(model, glm::vec3(2, 2, 2));

	GLint modelLoc = glGetUniformLocation(this->shadowmapShader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 180, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawTexturedPlane()
{
	UseShader(this->shadowmapShader);

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
	GLint modelLoc = glGetUniformLocation(this->shadowmapShader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
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
	RenderEngine& app = Demo();
	app.Start("Multiple Lighting Demo", 800, 600, false, false);
}