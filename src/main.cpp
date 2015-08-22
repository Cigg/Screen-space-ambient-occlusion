// Include standard headers
#include <cstdio>
#include <cstdlib>

#include <cassert>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cstring>
#include <time.h>

//#include <SOIL.h>
#include <FreeImagePlus.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "tiny_obj_loader.h"
#include "controls.h"
#include "shader.h"
#include "texture.h"
#include "gbuffer.h"

#define KERNEL_SIZE 32
#define KERNEL_RADIUS 4
#define BLUR_SIZE 4

using namespace glm;

struct DrawMesh {
	GLuint vertex_array;
	GLuint vbo_indices;
	GLuint num_indices;
	GLuint vbo_vertices;
	GLuint vbo_normals;
	GLuint vbo_texcoords;
	int material_id;
};

GLFWwindow* window;

unsigned int screenWidth = 1024;
unsigned int screenHeight = 768;

// --------------------------------------------------------------------
// Global "pointer"-variables to OpenGL objects and locations and stuff
// --------------------------------------------------------------------
// Shader programs
GLuint geometryShader;
GLuint ssaoShader;
GLuint blurShader;
GLuint lightingShader;

// Geometry shader locations
GLuint matrixID;
GLuint viewMatrixID;
GLuint modelMatrixID;
GLuint hasTextureMaskID;
GLuint diffuseTextureID;
GLuint diffuseTextureMaskID;

// SSAO shader locations
GLuint ssao_normalTextureID;
GLuint ssao_depthTextureID;
GLuint ssao_projectionID;
GLuint ssao_inverseProjectionID;
GLuint ssao_ssaoKernelID;
GLuint ssao_noiseTextureID;
GLuint ssao_noiseScaleID;
GLuint ssao_kernelSize;
GLuint ssao_kernelRadius;
GLuint ssao_isTurnedOn;

// Blur shader locations
GLuint blur_ssaoTextureID;
GLuint blur_blurSizeID;
GLuint blur_screenSizeID;
GLuint blur_depthTextureID;

// Light shader locations
GLuint light_worldPositionTextureID;
GLuint light_colorTextureID;
GLuint light_normalTextureID;
GLuint light_depthTextureID;
GLuint light_ssaoTextureID;
GLuint light_lightDirectionID;
GLuint light_eyePositionID;
GLuint light_inverseViewMatrixID;
GLuint light_useTexture;
GLuint light_screenSizeID;

// Render to texture buffer objects
GLuint ssaoFrameBuffer;
GLuint blurFrameBuffer;
GLuint ssaoTexture;
GLuint blurTexture;
GLuint quadVertexBuffer;
GLuint quadVertexArray;
// ---------------------------------------------------------------------

GBuffer gbuffer;
// Data structures to store meshes and textures
std::vector<DrawMesh> drawMeshes;
std::map<int, GLuint> diffuseTextures;
std::map<int, GLuint> diffuseMasks;

// Stuff needed to in SSAO
GLfloat ssaoKernel[3 * KERNEL_SIZE];
GLfloat ssaoRotationNoise[3 * KERNEL_RADIUS * KERNEL_RADIUS];
GLfloat noiseScale[2];
GLuint ssaoRotationNoiseTexture;


void printGLError(GLenum glError) {
	switch (glError) {
        case GL_INVALID_ENUM:
            std::cout << "Invalid enum." << std::endl;
            break;

        case GL_INVALID_VALUE:
            std::cout << "Invalid value." << std::endl;
            break;

        case GL_INVALID_OPERATION:
            std::cout << "Invalid operation." << std::endl;

        default:
            std::cout << "Unrecognised GLenum." << std::endl;
            break;
    }
}

float randomFloat(float a, float b) {
    float random = ((float) std::rand()) / (float) RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}

bool loadObjects(std::vector<tinyobj::shape_t> &shapes,
				 std::vector<tinyobj::material_t> &materials) {

	std::string inputfile = "../data/crytek-sponza/sponza.obj";
	std::string materialBasePath = "../data/crytek-sponza/";

	std::string err = tinyobj::LoadObj(shapes, materials, inputfile.c_str(), materialBasePath.c_str(), 0.01);

	if (!err.empty()) {
		std::cerr << err << std::endl;
		return false;
	}

	std::cout << "# of shapes    : " << shapes.size() << std::endl;
	std::cout << "# of materials : " << materials.size() << std::endl;

	// for (size_t i = 0; i < shapes.size(); i++) {
	//   printf("shape[%ld].name = %s\n", i, shapes[i].name.c_str());
	//   printf("Size of shape[%ld].indices: %ld\n", i, shapes[i].mesh.indices.size());
	//   printf("Size of shape[%ld].material_ids: %ld\n", i, shapes[i].mesh.material_ids.size());
	//   assert((shapes[i].mesh.indices.size() % 3) == 0);
	//   for (size_t f = 0; f < shapes[i].mesh.indices.size() / 3; f++) {
	//     printf("  idx[%ld] = %d, %d, %d. mat_id = %d\n", f, shapes[i].mesh.indices[3*f+0], shapes[i].mesh.indices[3*f+1], shapes[i].mesh.indices[3*f+2], shapes[i].mesh.material_ids[f]);
	//   }

	//   printf("shape[%ld].vertices: %ld\n", i, shapes[i].mesh.positions.size());
	//   assert((shapes[i].mesh.positions.size() % 3) == 0);
	// for (size_t v = 0; v < shapes[i].mesh.positions.size() / 3; v++) {
	//     printf("  v[%ld] = (%f, %f, %f)\n", v,
	// 		shapes[i].mesh.positions[3*v+0],
	// 		shapes[i].mesh.positions[3*v+1],
	// 		shapes[i].mesh.positions[3*v+2]);
	// 	}
	// }

	for (size_t i = 0; i < materials.size(); i++) {
		printf("material[%ld].name = %s\n", i, materials[i].name.c_str());
		printf("  material.Ka = (%f, %f ,%f)\n", materials[i].ambient[0], materials[i].ambient[1], materials[i].ambient[2]);
		printf("  material.Kd = (%f, %f ,%f)\n", materials[i].diffuse[0], materials[i].diffuse[1], materials[i].diffuse[2]);
		printf("  material.Ks = (%f, %f ,%f)\n", materials[i].specular[0], materials[i].specular[1], materials[i].specular[2]);
		printf("  material.Tr = (%f, %f ,%f)\n", materials[i].transmittance[0], materials[i].transmittance[1], materials[i].transmittance[2]);
		printf("  material.Ke = (%f, %f ,%f)\n", materials[i].emission[0], materials[i].emission[1], materials[i].emission[2]);
		printf("  material.Ns = %f\n", materials[i].shininess);
		printf("  material.Ni = %f\n", materials[i].ior);
		printf("  material.dissolve = %f\n", materials[i].dissolve);
		printf("  material.illum = %d\n", materials[i].illum);
		printf("  material.map_Ka = %s\n", materials[i].ambient_texname.c_str());
		printf("  material.map_Kd = %s\n", materials[i].diffuse_texname.c_str());
		printf("  material.map_Ks = %s\n", materials[i].specular_texname.c_str());
		printf("  material.map_Ns = %s\n", materials[i].normal_texname.c_str());
		std::map<std::string,std::string>::const_iterator it(materials[i].unknown_parameter.begin());
		std::map<std::string,std::string>::const_iterator itEnd(materials[i].unknown_parameter.end());
		for (; it != itEnd; it++) {
			printf("  material.%s = %s\n", it->first.c_str(), it->second.c_str());
		}
		printf("\n");
	}

	return true;
}

DrawMesh initMesh(tinyobj::mesh_t &mesh) { 
	DrawMesh out;

	glGenVertexArrays(1, &(out.vertex_array));
	glBindVertexArray(out.vertex_array);

	assert((mesh.positions.size() % 3) == 0);

	// Load VBOs

	// Vertices
	glGenBuffers(1, &(out.vbo_vertices));
	glBindBuffer(GL_ARRAY_BUFFER, out.vbo_vertices);
	glBufferData(GL_ARRAY_BUFFER, mesh.positions.size() * sizeof(float), &mesh.positions[0], GL_STATIC_DRAW);

	// Texture coordinates
	glGenBuffers(1, &(out.vbo_texcoords));
	glBindBuffer(GL_ARRAY_BUFFER, out.vbo_texcoords);
	glBufferData(GL_ARRAY_BUFFER, mesh.texcoords.size() * sizeof(float), &mesh.texcoords[0], GL_STATIC_DRAW);

	// Normals
	glGenBuffers(1, &(out.vbo_normals));
	glBindBuffer(GL_ARRAY_BUFFER, out.vbo_normals);
	glBufferData(GL_ARRAY_BUFFER, mesh.normals.size() * sizeof(float), &mesh.normals[0], GL_STATIC_DRAW);

	// Indices
	glGenBuffers(1, &(out.vbo_indices));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, out.vbo_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(unsigned int), &mesh.indices[0] , GL_STATIC_DRAW);

	// Unbind vertex array
	glBindVertexArray(0);

	out.num_indices = mesh.indices.size();
	out.material_id = mesh.material_ids[0];

	return out;
}

bool initGL() {
	// ------------------------------------------------------------------------------------
	//
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(screenWidth, screenHeight, "SSAO", NULL, NULL);
	if (window == NULL){
		fprintf(stderr, "Failed to open GLFW window.\n");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);

	glewExperimental = GL_TRUE;
	// Initialize GLEW
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return false;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// // Enable blending
	// glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	return true;
}

void geometryPass() {
	// Compute the MVP matrix from keyboard and mouse input
	computeMatricesFromInputs();
	glm::mat4 projectionMatrix = getProjectionMatrix();
	glm::mat4 viewMatrix = getViewMatrix();
	glm::mat4 modelMatrix = glm::mat4(1.0);
	glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(matrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMatrix[0][0]);

	for(size_t i = 0; i < drawMeshes.size(); i++) {
		// Bind diffuse texture
		glActiveTexture(GL_TEXTURE0);
		// Get the texture associated with the material_id
		glBindTexture(GL_TEXTURE_2D, diffuseTextures[drawMeshes[i].material_id]);
		glUniform1i(diffuseTextureID, 0);

		// Bind diffuse texture mask if the object has one
		if(diffuseMasks[drawMeshes[i].material_id] != 0) {
			glUniform1i(hasTextureMaskID, 1);
			glActiveTexture(GL_TEXTURE1);
			// Get the texture associated with the material_id
			glBindTexture(GL_TEXTURE_2D, diffuseMasks[drawMeshes[i].material_id]);
			glUniform1i(diffuseTextureMaskID, 1);
		
		}
		else {
			glUniform1i(hasTextureMaskID, 0);	
		}

		glBindVertexArray(drawMeshes[i].vertex_array);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, drawMeshes[i].vbo_vertices);
		glVertexAttribPointer(
			0,                  // attribute
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, drawMeshes[i].vbo_texcoords);
		glVertexAttribPointer(
			1,                                // attribute
			2,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// 3rd attribute buffer : normals
		glEnableVertexAttribArray(2);
		glBindBuffer(GL_ARRAY_BUFFER, drawMeshes[i].vbo_normals);
		glVertexAttribPointer(
			2,                                // attribute
			3,                                // size
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Index buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, drawMeshes[i].vbo_indices);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			drawMeshes[i].num_indices,    // count
			GL_UNSIGNED_INT,   // type
			(void*)0           // element array buffer offset
		);

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
		glDisableVertexAttribArray(2);
	}

	glBindVertexArray(0);
}

void lightingPass() {
	glUseProgram(lightingShader);
	// SET UNIFORMS
	// glUniform1i(worldPositionTextureID, 0);
	glUniform1i(light_colorTextureID, 1);
	glUniform1i(light_normalTextureID, 2);
	glUniform1i(light_depthTextureID, 3);
	glUniform1i(light_ssaoTextureID, 4);
	glUniform3f(light_lightDirectionID, 0.0, 3.0, 1.0);
	glm::vec3 camPos = getPosition(); 
	glUniform3f(light_eyePositionID, camPos.x, camPos.y, camPos.z);
	glUniform1f(light_useTexture, isTexturesEnabled());
	glUniform2f(light_screenSizeID, screenWidth, screenHeight);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, blurTexture);

	glm::mat4 viewMatrix = getViewMatrix();
	glm::mat4 inverseViewMatrix = glm::inverse(viewMatrix);
	glUniformMatrix4fv(light_inverseViewMatrixID, 1, GL_FALSE, &inverseViewMatrix[0][0]);

	// 1rst attribute buffer : vertices
	glBindVertexArray(quadVertexArray);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

	glDisableVertexAttribArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ssaoPass() {
	glUseProgram(ssaoShader);

	glm::mat4 projectionMatrix = getProjectionMatrix();
	glm::mat4 inverseProjectionMatrix = glm::inverse(projectionMatrix);
	
	// Set uniforms
	glUniform1i(ssao_normalTextureID, 2);
	glUniform1i(ssao_depthTextureID, 3);
	glUniform1i(ssao_noiseTextureID, 4);
	glUniform3fv(ssao_ssaoKernelID, KERNEL_SIZE, ssaoKernel);
	glUniform2f(ssao_noiseScaleID, noiseScale[0], noiseScale[1]);
	glUniformMatrix4fv(ssao_projectionID, 1, GL_FALSE, &projectionMatrix[0][0]);
	glUniformMatrix4fv(ssao_inverseProjectionID, 1, GL_FALSE, &inverseProjectionMatrix[0][0]);
	glUniform1i(ssao_kernelSize, KERNEL_SIZE);
	glUniform1i(ssao_kernelRadius, KERNEL_RADIUS);
	glUniform1f(ssao_isTurnedOn, isSSAOEnabled());

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ssaoRotationNoiseTexture);

	// 1rst attribute buffer : vertices
	glBindVertexArray(quadVertexArray);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

//	glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void blurPass() {
	glUseProgram(blurShader);

	// SET UNIFORMS
	glUniform1i(blur_depthTextureID, 3);
	glUniform1i(blur_ssaoTextureID, 4);
	glUniform2f(blur_screenSizeID, screenWidth, screenHeight);
	glUniform1f(blur_blurSizeID, BLUR_SIZE);
	
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, ssaoTexture);

	// 1rst attribute buffer : vertices
	glBindVertexArray(quadVertexArray);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Draw the triangles !
	glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

	// glBindTexture(GL_TEXTURE_2D, 0);
	glDisableVertexAttribArray(0);
}

int main(void)
{
	// Init OpenGL
	if(initGL()) {
		std::cout << "OpenGL initialized successfully" << std::endl;
	}
	else {
		std::cout << "OpenGL not initialized successfully. Exiting." << std::endl;
		return -1;
	}

	// Check for errors
	GLenum glError = glGetError();
    if(glError) {
        std::cout << "Error after initGL: ";
    	printGLError(glError);
    }

	// Initialise the FreeImage library
	FreeImage_Initialise(true);

	// Create vectors to load objects into
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	
	// Load objects
	if(loadObjects(shapes, materials)) {
		std::cout << "Objects loaded successfully" << std::endl;
	}
	else {
		std::cout << "Objects not loaded. Exiting." << std::endl;
		return -1;
	}

	// Load VBOs
	for (size_t i = 0; i < shapes.size(); i++) {
		drawMeshes.push_back(initMesh(shapes[i].mesh));
	}

	// Load diffuse textures
	for(size_t i = 0; i < materials.size(); i++) {
		// Load the texture
		std::string dir = "../data/crytek-sponza/";
		std::string tex = materials[i].diffuse_texname;
		std::replace( tex.begin(), tex.end(), '\\', '/'); // replace all '\' to '/'
		std::string path =  dir + tex;
		// Load the image using trilinear filtering via mipmaps for minification and GL_LINEAR for magnification
		GLuint textureID = loadTexture(path, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		std::cout << "Loading texture with path: " << path.c_str() << ", textureID: " << textureID << std::endl;
		diffuseTextures[i] = textureID;
	}

	// Load diffuse texture masks
	for(size_t i = 0; i < materials.size(); i++) {
		// Load the texture
		std::string dir = "../data/crytek-sponza/";
		std::string tex = materials[i].unknown_parameter["map_d"];
		if(tex != "") {
			std::replace( tex.begin(), tex.end(), '\\', '/'); // replace all '\' to '/'
			std::string path =  dir + tex;
			// Load the image using trilinear filtering via mipmaps for minification and GL_LINEAR for magnification
			GLuint textureID = loadTexture(path, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
			diffuseMasks[i] = textureID;
		}
	}

	// Init geometry buffer (FBO and textures)
    if (!gbuffer.Init(screenWidth, screenHeight)) {
        return -1;
    }

	// Create and compile our GLSL program from the geometry shaders
	geometryShader = LoadShaders( "../shaders/GeometryPass.vert", "../shaders/GeometryPass.frag" );
	glUseProgram(geometryShader);
	// Get uniform locations
	matrixID = glGetUniformLocation(geometryShader, "MVP");
	viewMatrixID = glGetUniformLocation(geometryShader, "V");
	modelMatrixID = glGetUniformLocation(geometryShader, "M");
	hasTextureMaskID = glGetUniformLocation(geometryShader, "HasTextureMask");
	diffuseTextureID  = glGetUniformLocation(geometryShader, "DiffuseTexture");
	diffuseTextureMaskID  = glGetUniformLocation(geometryShader, "DiffuseTextureMask");

	// Create and compile our GLSL program from the ssao shaders
	ssaoShader = LoadShaders("../shaders/Quad.vert", "../shaders/SSAO.frag");
	// Get uniform locations
	ssao_normalTextureID = glGetUniformLocation(ssaoShader, "NormalTexture");
	ssao_depthTextureID = glGetUniformLocation(ssaoShader, "DepthTexture");
	ssao_inverseProjectionID = glGetUniformLocation(ssaoShader, "InverseProjectionMatrix");
	ssao_projectionID = glGetUniformLocation(ssaoShader, "ProjectionMatrix");
	ssao_ssaoKernelID = glGetUniformLocation(ssaoShader, "SSAOKernel");
	ssao_noiseTextureID = glGetUniformLocation(ssaoShader, "NoiseTexture");
	ssao_noiseScaleID = glGetUniformLocation(ssaoShader, "NoiseScale");
	ssao_kernelSize = glGetUniformLocation(ssaoShader, "KernelSize");
	ssao_kernelRadius = glGetUniformLocation(ssaoShader, "KernelRadius");
	ssao_isTurnedOn = glGetUniformLocation(ssaoShader, "IsTurnedOn");

	// Create and compile our GLSL program from the blur shaders
	blurShader = LoadShaders("../shaders/Quad.vert", "../shaders/Blur.frag");
	// Get uniform locations
	blur_ssaoTextureID = glGetUniformLocation(blurShader, "SSAOTexture");
	blur_blurSizeID = glGetUniformLocation(blurShader, "BlurSize");
	blur_screenSizeID = glGetUniformLocation(blurShader, "ScreenSize");
	blur_depthTextureID = glGetUniformLocation(blurShader, "DepthTexture");

	// Create and compile our GLSL program from the lighting shaders
	lightingShader = LoadShaders("../shaders/Quad.vert", "../shaders/LightPass.frag");
	// Get uniform locations
	light_worldPositionTextureID = glGetUniformLocation(lightingShader, "WorldPositionTexture");
	light_colorTextureID = glGetUniformLocation(lightingShader, "ColorTexture");
	light_normalTextureID = glGetUniformLocation(lightingShader, "NormalTexture");
	light_ssaoTextureID = glGetUniformLocation(lightingShader, "SSAOTexture");
	light_depthTextureID = glGetUniformLocation(lightingShader, "DepthTexture");
	light_lightDirectionID = glGetUniformLocation(lightingShader, "LightDirection_worldspace");
	light_eyePositionID = glGetUniformLocation(lightingShader, "EyePosition_worldspace");
	light_inverseViewMatrixID = glGetUniformLocation(lightingShader, "InverseViewMatrix");
	light_useTexture = glGetUniformLocation(lightingShader, "UseTexture");
	light_screenSizeID = glGetUniformLocation(lightingShader, "ScreenSize");

	// Create a framebuffer and texture for the SSAO pass
	glGenFramebuffers(1, &ssaoFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFrameBuffer);
    glGenTextures(1, &ssaoTexture);
	glBindTexture(GL_TEXTURE_2D, ssaoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ssaoTexture, 0);

    // Create a framebuffer and texture for the blur pass
    glGenFramebuffers(1, &blurFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, blurFrameBuffer);
    glGenTextures(1, &blurTexture);
	glBindTexture(GL_TEXTURE_2D, blurTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, screenWidth, screenHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, blurTexture, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Create a quad and put it in a VBO
	static const GLfloat quadVertexBufferData[] = { 
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

	glGenBuffers(1, &quadVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quadVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertexBufferData), quadVertexBufferData, GL_STATIC_DRAW);
	glGenVertexArrays(1, &quadVertexArray);

	// For FPS computation
	double lastTime = glfwGetTime();
	int fpsCount = 0;
	int fps = 0;

	// Initialize SSAO kernel and rotation noise
	std::srand(time(NULL));
	for(unsigned int i = 0; i < KERNEL_SIZE; i++) {
		glm::vec3 randVec = glm::vec3( randomFloat(-1.0, 1.0),
									   randomFloat(-1.0, 1.0),
									   randomFloat(0.0, 1.0));

		glm::normalize(randVec);

		float scale = float(i/3) / (float)KERNEL_SIZE;
		scale = 0.1 + scale*scale*0.9;

		randVec *= scale;

		ssaoKernel[i*3] = randVec.x;
		ssaoKernel[i*3 + 1] = randVec.y;
		ssaoKernel[i*3 + 2] = randVec.z;
	}


	for(unsigned int i = 0; i < KERNEL_RADIUS*KERNEL_RADIUS; i++) {
		glm::vec3 randVec = glm::vec3( randomFloat(-1.0, 1.0),
							   		   randomFloat(-1.0, 1.0),
							   		   0.0);
		glm::normalize(randVec);

		ssaoRotationNoise[i*3] = randVec.x;
		ssaoRotationNoise[i*3 + 1] = randVec.y;
		ssaoRotationNoise[i*3 + 2] = randVec.z;
	}

	noiseScale[0] = screenWidth / 4;
	noiseScale[1] = screenHeight / 4;

	// Create SSAO rotation noise texture
	glGenTextures(1, &ssaoRotationNoiseTexture);
	glBindTexture(GL_TEXTURE_2D, ssaoRotationNoiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, KERNEL_RADIUS, KERNEL_RADIUS, 0, GL_RGB, GL_FLOAT, ssaoRotationNoise);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Main loop
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0) {

		// Measure speed
		double currentTime = glfwGetTime();
		fpsCount++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
			// printf and reset
			fps = fpsCount;
			fpsCount = 0;
			lastTime += 1.0;

			// Updating the title every frame can be bad for the FPS on linux
			// Print positiong and FPS in the console instead 
			#ifdef __linux__
				// Update title with current position and FPS
				glm::vec3 camPos = getPosition(); 
				std::string str = "Pos: " + std::to_string(camPos[0]) + "," + std::to_string(camPos[1]) + "," + std::to_string(camPos[2]) +
								  ". FPS: " + std::to_string(fps);
				std::cout << str << std::endl;					
			#endif
		}

		#ifndef __linux__
			// Update title with current position and FPS
			glm::vec3 camPos = getPosition(); 
			std::string str = "Pos: " + std::to_string(camPos[0]) + "," + std::to_string(camPos[1]) + "," + std::to_string(camPos[2]) +
							  ". FPS: " + std::to_string(fps);
			glfwSetWindowTitle(window, str.c_str());
		#endif

		// Render geometry to the framebuffer		
		gbuffer.BindForWriting();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(geometryShader);		
		geometryPass();

		//
		gbuffer.BindTextures();

		// Render ssao texture
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFrameBuffer);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ssaoPass();

		// Render blurred ssao texture
		glBindFramebuffer(GL_FRAMEBUFFER, blurFrameBuffer);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		blurPass();

		// Render to the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		lightingPass();

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	//// Cleanup VBO and shader
	//for(size_t i = 0; drawMeshes.size(); i++) {
	//	glDeleteBuffers(1, &(drawMeshes[i].vbo_vertices));
	//	glDeleteBuffers(1, &(drawMeshes[i].vbo_texcoords));
	//	glDeleteBuffers(1, &(drawMeshes[i].vbo_normals));
	//	glDeleteBuffers(1, &(drawMeshes[i].vbo_normals));
	//	glDeleteProgram(geometryShader);
	//	glDeleteVertexArrays(1, &(drawMeshes[i].vertex_array));
	//	glDeleteVertexArrays(1, &quadVertexArray);
	//}

	// Cleanup textures
	for (size_t i = 0; i < diffuseTextures.size(); i++) {
		glDeleteTextures(1, &diffuseTextures[i]);
	}

	for(size_t i = 0; i < diffuseMasks.size(); i++) {
		glDeleteTextures(1, &diffuseMasks[i]);
	}

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

