// Include standard headers
#include <cstdio>
#include <cstdlib>

#include <cassert>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <fstream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <SOIL.h>

#include "controls.h"
#include "tiny_obj_loader.h"
#include "shader.h"

using namespace glm;

GLFWwindow* window;

unsigned int screenWidth = 1024;
unsigned int screenHeight = 768;

struct draw_mesh {
	GLuint vertex_array;
	GLuint vbo_indices;
	GLuint num_indices;
	GLuint vbo_vertices;
	GLuint vbo_normals;
	GLuint vbo_texcoords;
	int material_id;
};

GLuint LoadTGAFile(const char *filename)
{
	unsigned char imageTypeCode;
    short int imageWidth;
    short int imageHeight;
    unsigned char bitCount;
    unsigned char *imageData;

    unsigned char ucharBad;
    short int sintBad;
    long imageSize;
    int colorMode;
    unsigned char colorSwap;

    // Open the TGA file.
    FILE *filePtr = fopen(filename, "rb");
    if (filePtr == NULL) {
    	printf("%s could not be opened.\n", filename);
    	return 0;
    }

    // Read the two first bytes we don't need.
    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

    // Which type of image gets stored in imageTypeCode.
    fread(&imageTypeCode, sizeof(unsigned char), 1, filePtr);

    // For our purposes, the type code should be 2 (uncompressed RGB image)
    // or 3 (uncompressed black-and-white images).
    if (imageTypeCode != 2 && imageTypeCode != 3)
    {
    	printf("%s wrong TGA type.\n", filename);
        fclose(filePtr);
        return false;
    }

    //std::cout << "dataTypeCode: " << (unsigned int)imageTypeCode << std::endl;

    // Read 13 bytes of data we don't need.
    fread(&sintBad, sizeof(short int), 1, filePtr);
    fread(&sintBad, sizeof(short int), 1, filePtr);
    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);
    fread(&sintBad, sizeof(short int), 1, filePtr);
    fread(&sintBad, sizeof(short int), 1, filePtr);
    
    // Read the image's width and height.
    fread(&imageWidth, sizeof(short int), 1, filePtr);
    fread(&imageHeight, sizeof(short int), 1, filePtr);
    //std::cout << "imageWidth: " << imageWidth << std::endl;
    //std::cout << "imageHeight: " << imageHeight << std::endl;

    // Read the bit depth.
    fread(&bitCount, sizeof(unsigned char), 1, filePtr);
    //std::cout << "bitCount: " << (unsigned int)bitCount << std::endl;

    // Read one byte of data we don't need.
    fread(&ucharBad, sizeof(unsigned char), 1, filePtr);

    // Color mode -> 3 = BGR, 4 = BGRA.
    colorMode = bitCount / 8;
    imageSize = imageWidth * imageHeight * colorMode;

    // Allocate memory for the image data.
    imageData = (unsigned char*)malloc(sizeof(unsigned char)*imageSize);

    // Read the image data.
    fread(imageData, sizeof(unsigned char), imageSize, filePtr);

    // Change from BGR to RGB so OpenGL can read the image data.
    for (int imageIdx = 0; imageIdx < imageSize; imageIdx += colorMode)
    {
        colorSwap = imageData[imageIdx];
        imageData[imageIdx] = imageData[imageIdx + 2];
        imageData[imageIdx + 2] = colorSwap;
    }

   	// Everything is in memory now, the file wan be closed
    fclose(filePtr);
   
	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, imageData);

   	// OpenGL has now copied the data. Free our own version
	//delete [] imageData;
	free(imageData);

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
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

draw_mesh initMesh(tinyobj::mesh_t &mesh) { 
	draw_mesh out;

	glGenVertexArrays(1, &(out.vertex_array));
	glBindVertexArray(out.vertex_array);

	//std::cout << "size: " mesh.positions.size() << std::endl;
	// printf("shape[%ld].positions: %ld\n", i, mesh.positions.size());
	// printf("shape[%ld].indices: %ld\n", i, mesh.indices.size());
	// printf("shape[%ld].texcoords: %ld\n", i, mesh.texcoords.size());
	// printf("shape[%ld].normals: %ld\n", i, mesh.normals.size());
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
	// Initialise GLFW
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

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 
	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);
	
	return true;
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

	std::vector<draw_mesh> draw_meshes;
	for (size_t i = 0; i < shapes.size(); i++) {
		draw_meshes.push_back(initMesh(shapes[i].mesh));
	}

	std::map<int, GLuint> diffuse_textures;
	for(size_t i = 0; i < materials.size(); i++) {
		// Load the texture
		std::string dir = "../data/crytek-sponza/";
		std::string tex = materials[i].diffuse_texname;
		std::replace( tex.begin(), tex.end(), '\\', '/'); // replace all '\' to '/'
		std::string path =  dir + tex;
		GLuint textureID = LoadTGAFile(path.c_str());
		diffuse_textures[i] = textureID;
	}

	std::map<int, GLuint> diffuse_masks;
	for(size_t i = 0; i < materials.size(); i++) {
		// Load the texture
		std::string dir = "../data/crytek-sponza/";
		std::string tex = materials[i].unknown_parameter["map_d"];
		if(tex != "") {
			std::replace( tex.begin(), tex.end(), '\\', '/'); // replace all '\' to '/'
			std::string path =  dir + tex;
			GLuint textureID = LoadTGAFile(path.c_str());
			diffuse_masks[i] = textureID;
		}
	}

	// Create and compile our GLSL program from the shaders
	GLuint shader = LoadShaders( "../shaders/StandardShading.vertexshader", "../shaders/StandardShading.fragmentshader" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(shader, "MVP");
	GLuint ViewMatrixID = glGetUniformLocation(shader, "V");
	GLuint ModelMatrixID = glGetUniformLocation(shader, "M");

	// Get a handle for our "LightPosition" uniform
	glUseProgram(shader);
	GLuint LightID = glGetUniformLocation(shader, "LightPosition_worldspace");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint Texture1ID  = glGetUniformLocation(shader, "diffuseTexture");
	GLuint Texture2ID  = glGetUniformLocation(shader, "diffuseTextureMask");

	// For speed computation
	double lastTime = glfwGetTime();
	int fpsCount = 0;
	int fps = 0;

	
	// Main loop
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0) {

		// Draw stuff

		// Measure speed
		double currentTime = glfwGetTime();
		fpsCount++;
		if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
			// printf and reset
			fps = fpsCount;
			fpsCount = 0;
			lastTime += 1.0;
		}

		glm::vec3 camPos = getPosition(); 
		std::string str = "Pos: " + std::to_string(camPos[0]) + "," + std::to_string(camPos[1]) + "," + std::to_string(camPos[2]) +
						  ". FPS: " + std::to_string(fps);
		glfwSetWindowTitle(window, str.c_str());

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(shader);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 ProjectionMatrix = getProjectionMatrix();
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::mat4 ModelMatrix = glm::mat4(1.0);
		glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

		glm::vec3 lightPos = glm::vec3(4,4,4);
		glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);


		for(size_t i = 0; i < draw_meshes.size(); i++) {
			// Bind diffuse texture
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, diffuse_textures[draw_meshes[i].material_id]); // Get the texture associated with the material_id
			// Set our "myTextureSampler" sampler to user Texture Unit 0
			glUniform1i(Texture1ID, 0);

			// Bind diffuse texture mask
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, diffuse_masks[draw_meshes[i].material_id]); // Get the texture associated with the material_id
			// Set our "myTextureSampler" sampler to user Texture Unit 0
			glUniform1i(Texture2ID, 1);

			glBindVertexArray(draw_meshes[i].vertex_array);

			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, draw_meshes[i].vbo_vertices);
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
			glBindBuffer(GL_ARRAY_BUFFER, draw_meshes[i].vbo_texcoords);
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
			glBindBuffer(GL_ARRAY_BUFFER, draw_meshes[i].vbo_normals);
			glVertexAttribPointer(
				2,                                // attribute
				3,                                // size
				GL_FLOAT,                         // type
				GL_FALSE,                         // normalized?
				0,                                // stride
				(void*)0                          // array buffer offset
			);

			// Index buffer
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, draw_meshes[i].vbo_indices);

			// Draw the triangles !
			glDrawElements(
				GL_TRIANGLES,      // mode
				draw_meshes[i].num_indices,    // count
				GL_UNSIGNED_INT,   // type
				(void*)0           // element array buffer offset
			);

			glDisableVertexAttribArray(0);
			glDisableVertexAttribArray(1);
			glDisableVertexAttribArray(2);
		}

		glBindVertexArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// Cleanup VBO and shader
	for(size_t i = 0; draw_meshes.size(); i++) {
		glDeleteBuffers(1, &(draw_meshes[i].vbo_vertices));
		glDeleteBuffers(1, &(draw_meshes[i].vbo_texcoords));
		glDeleteBuffers(1, &(draw_meshes[i].vbo_normals));
		glDeleteBuffers(1, &(draw_meshes[i].vbo_normals));
		glDeleteProgram(shader);
		glDeleteVertexArrays(1, &(draw_meshes[i].vertex_array));
	}

	// Cleanup textures
	for(size_t i = 0; diffuse_textures.size(); i++) {
		glDeleteTextures(1, &diffuse_textures[i]);
	}

	for(size_t i = 0; diffuse_masks.size(); i++) {
		glDeleteTextures(1, &diffuse_masks[i]);
	}
	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}

