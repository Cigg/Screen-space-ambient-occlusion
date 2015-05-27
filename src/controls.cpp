// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.h"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

// Initial position : on +Z 
glm::vec3 position = glm::vec3( 7, 10, -0.5 ); 
// Initial horizontal angle : toward -X
float horizontalAngle = 4.71f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;

enum SSAO_STATUS {
	SSAO_OFF,
	SSAO_ON
};

int ssaoStatus = SSAO_OFF;

bool isPressed = false;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}
glm::vec3 getPosition() {
	return position;
}

int isSSAOOn() {
	return ssaoStatus;
}

void computeMatricesFromInputs(){

	unsigned int screenWidth = 1024;
	unsigned int screenHeight = 768;

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	glfwSetCursorPos(window, screenWidth/2, screenHeight/2);

	// Compute new orientation
	horizontalAngle += mouseSpeed * float(screenWidth/2 - xpos );
	verticalAngle   += mouseSpeed * float( screenHeight/2 - ypos );

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	glm::vec3 direction(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
		position -= right * deltaTime * speed;
	}
	// Turn off/on SSAO
	if (glfwGetKey( window, GLFW_KEY_O ) == GLFW_PRESS && !isPressed){
		isPressed = true;
		if(ssaoStatus == SSAO_ON)
			ssaoStatus = SSAO_OFF;
		else
			ssaoStatus = SSAO_ON;
	}
	if (glfwGetKey( window, GLFW_KEY_O ) == GLFW_RELEASE) {
		isPressed = false;
	}

	float FoV = initialFoV;

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(FoV, 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								position+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}