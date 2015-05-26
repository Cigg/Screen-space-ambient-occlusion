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

unsigned int screenWidth = 1280;
unsigned int screenHeight = 1024;
//unsigned int frameBufferSize = 1024;	

// --------------------------------------------------------------------
// Global "pointer"-variables to OpenGL objects and locations and stuff
// --------------------------------------------------------------------

// Shader programs
GLuint geometryShader;
GLuint lightingShader;

// Geometry shader locations
GLuint MatrixID;
GLuint ViewMatrixID;
GLuint ModelMatrixID;
GLuint HasTextureMaskID;
GLuint diffuseTextureID;
GLuint diffuseTextureMaskID;

// Light shader locations
GLuint worldPositionTextureID;
GLuint colorTextureID;
GLuint normalTextureID;
GLuint depthTextureID;
GLuint lightDirectionID;
GLuint eyePositionID;
GLuint projectionID;
GLuint inverseProjectionID;

// Render to texture buffer objects
GLuint quad_vertexbuffer;
GLuint quad_vertex_array;

// --------------------------------------------------------------------
GBuffer gbuffer;
std::vector<DrawMesh> drawMeshes;
std::map<int, GLuint> diffuseTextures;
std::map<int, GLuint> diffuseMasks;

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
	window = glfwCreateWindow(screenWidth, screenHeight, "Test", NULL, NULL);
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
	glEnable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	return true;
}

void drawScene() {
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use our shader
	glUseProgram(geometryShader);

	// Compute the MVP matrix from keyboard and mouse input
	computeMatricesFromInputs();
	glm::mat4 projectionMatrix = getProjectionMatrix();
	glm::mat4 viewMatrix = getViewMatrix();
	glm::mat4 modelMatrix = glm::mat4(1.0);
	glm::mat4 MVP = projectionMatrix * viewMatrix * modelMatrix;

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &viewMatrix[0][0]);

	for(size_t i = 0; i < drawMeshes.size(); i++) {
		// Bind diffuse texture
		glActiveTexture(GL_TEXTURE0);
		// Get the texture associated with the material_id
		glBindTexture(GL_TEXTURE_2D, diffuseTextures[drawMeshes[i].material_id]);
		glUniform1i(diffuseTextureID, 0);

		// Bind diffuse texture mask if the object has one
		if(diffuseMasks[drawMeshes[i].material_id] != 0) {
			glUniform1i(HasTextureMaskID, 1);
			glActiveTexture(GL_TEXTURE1);
			// Get the texture associated with the material_id
			glBindTexture(GL_TEXTURE_2D, diffuseMasks[drawMeshes[i].material_id]);
			glUniform1i(diffuseTextureMaskID, 1);
		
		}
		else {
			glUniform1i(HasTextureMaskID, 0);	
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

	GLenum glError = glGetError();
    if(glError) {
        std::cout << "Error after initGL: ";
    	printGLError(glError);
    }

	// Initialise the FreeImage library
	// Note: Flag is whether we should load ONLY local (built-in) libraries, so
	// false means 'no, use external libraries also', and 'true' means - use built
	// in libs only, so it's like using the library as a static version of itself.
	FreeImage_Initialise(true);

	// Load objects
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	if(loadObjects(shapes, materials)) {
		std::cout << "Objects loaded successfully" << std::endl;
	}
	else {
		std::cout << "Objects not loaded. Exiting." << std::endl;
		return -1;
	}

	for (size_t i = 0; i < shapes.size(); i++) {
		drawMeshes.push_back(initMesh(shapes[i].mesh));
	}

	for(size_t i = 0; i < materials.size(); i++) {
		// Load the texture
		std::string dir = "../data/crytek-sponza/";
		std::string tex = materials[i].diffuse_texname;
		std::replace( tex.begin(), tex.end(), '\\', '/'); // replace all '\' to '/'
		std::string path =  dir + tex;
		//GLuint textureID = LoadTGAFile("../data/crytek-sponza/textures/spnza_bricks_a_diff.tga");
		// Load the image using trilinear filtering via mipmaps for minification and GL_LINEAR for magnification
		GLuint textureID = loadTexture(path, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		std::cout << "Loading texture with path: " << path.c_str() << ", textureID: " << textureID << std::endl;
		diffuseTextures[i] = textureID;
	}

	for(size_t i = 0; i < materials.size(); i++) {
		// Load the texture
		std::string dir = "../data/crytek-sponza/";
		std::string tex = materials[i].unknown_parameter["map_d"];
		if(tex != "") {
			std::replace( tex.begin(), tex.end(), '\\', '/'); // replace all '\' to '/'
			std::string path =  dir + tex;
			//GLuint textureID = LoadTGAFile(path.c_str());
			// Load the image using trilinear filtering via mipmaps for minification and GL_LINEAR for magnification
			GLuint textureID = loadTexture(path, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
			diffuseMasks[i] = textureID;
		}
	}

	std::cout << "Init geometry buffer" << std::endl;
	// Init geometry buffer (FBO and textures)
    if (!gbuffer.Init(screenWidth, screenHeight)) {
        return -1;
    }
    std::cout << "Geometry buffer initialized" << std::endl;


	// Create and compile our GLSL program from the shaders
	geometryShader = LoadShaders( "../shaders/GeometryPass.vert", "../shaders/GeometryPass.frag" );
	glUseProgram(geometryShader);

	// Get a handle for our "MVP" uniform
	MatrixID = glGetUniformLocation(geometryShader, "MVP");
	ViewMatrixID = glGetUniformLocation(geometryShader, "V");
	ModelMatrixID = glGetUniformLocation(geometryShader, "M");

	// Get a handle for other uniforms
	HasTextureMaskID = glGetUniformLocation(geometryShader, "HasTextureMask");
	
	// Get a handle for our texture uniforms
	diffuseTextureID  = glGetUniformLocation(geometryShader, "diffuseTexture");
	diffuseTextureMaskID  = glGetUniformLocation(geometryShader, "diffuseTextureMask");

	// The fullscreen quad's FBO
	static const GLfloat g_quad_vertex_buffer_data[] = { 
		-1.0f, -1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f,
		 1.0f, -1.0f, 0.0f,
		 1.0f,  1.0f, 0.0f,
	};

	glGenBuffers(1, &quad_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);
	glGenVertexArrays(1, &quad_vertex_array);

	lightingShader = LoadShaders( "../shaders/LightPass.vert", "../shaders/LightPass.frag" );
	worldPositionTextureID = glGetUniformLocation(lightingShader, "WorldPositionTexture");
	colorTextureID = glGetUniformLocation(lightingShader, "ColorTexture");
	normalTextureID = glGetUniformLocation(lightingShader, "NormalTexture");
	depthTextureID = glGetUniformLocation(lightingShader, "DepthTexture");
	lightDirectionID = glGetUniformLocation(lightingShader, "LightDirection_worldspace");
	eyePositionID = glGetUniformLocation(lightingShader, "EyePosition_worldspace");
	inverseProjectionID = glGetUniformLocation(lightingShader, "InverseProjectionMatrix");
	projectionID = glGetUniformLocation(lightingShader, "ProjectionMatrix");

	// For FPS computation
	double lastTime = glfwGetTime();
	int fpsCount = 0;
	int fps = 0;

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
		}

		// Update title with current position and FPS
		glm::vec3 camPos = getPosition(); 
		std::string str = "Pos: " + std::to_string(camPos[0]) + "," + std::to_string(camPos[1]) + "," + std::to_string(camPos[2]) +
						  ". FPS: " + std::to_string(fps);
		glfwSetWindowTitle(window, str.c_str());

		// ============================== RENDER GEOMETRY TO FRAMEBUFFER ===========================================
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gbuffer.BindForWriting();
		drawScene();

		// ========================= RENDER TO THE SCREEN ====================================================
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glUseProgram(lightingShader);

		gbuffer.SetReadBuffer(GBuffer::GBUFFER_TEXTURE_TYPE_NORMAL);
		// glActiveTexture(GL_TEXTURE0);
		// glBindTexture(GL_TEXTURE_2D, renderedTexture2);

		// SET UNIFORMS
		glUniform1i(worldPositionTextureID, 0);
		glUniform1i(colorTextureID, 1);
		glUniform1i(normalTextureID, 2);
		glUniform1i(depthTextureID, 3);
		glUniform3f(lightDirectionID, 0.0, 3.0, 1.0);
		glUniform3f(eyePositionID, camPos.x, camPos.y, camPos.z);

		glm::mat4 projectionMatrix = getProjectionMatrix();
		glm::mat4 inverseProjectionMatrix = glm::inverse(projectionMatrix);
		glUniformMatrix4fv(projectionID, 1, GL_FALSE, &projectionMatrix[0][0]);
		glUniformMatrix4fv(inverseProjectionID, 1, GL_FALSE, &inverseProjectionMatrix[0][0]);

		// 1rst attribute buffer : vertices
		glBindVertexArray(quad_vertex_array);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
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

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup VBO and shader
	for(size_t i = 0; drawMeshes.size(); i++) {
		glDeleteBuffers(1, &(drawMeshes[i].vbo_vertices));
		glDeleteBuffers(1, &(drawMeshes[i].vbo_texcoords));
		glDeleteBuffers(1, &(drawMeshes[i].vbo_normals));
		glDeleteBuffers(1, &(drawMeshes[i].vbo_normals));
		glDeleteProgram(geometryShader);
		glDeleteVertexArrays(1, &(drawMeshes[i].vertex_array));
		glDeleteVertexArrays(1, &quad_vertex_array);
	}

	// Cleanup textures
	for(size_t i = 0; diffuseTextures.size(); i++) {
		glDeleteTextures(1, &diffuseTextures[i]);
	}

	for(size_t i = 0; diffuseMasks.size(); i++) {
		glDeleteTextures(1, &diffuseMasks[i]);
	}
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

