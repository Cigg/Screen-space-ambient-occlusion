#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 VertexPosition_modelspace;
layout(location = 1) in vec2 VertexUV;
layout(location = 2) in vec3 VertexNormal_modelspace;

// Output data ; will be interpolated for each fragment.
out vec2 UV;
out vec3 Position_worldspace;
out vec3 Normal_worldspace;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;

void main() {
	// Output position of the vertex, in clip space : MVP * position
	gl_Position =  MVP * vec4(VertexPosition_modelspace,1);
	Position_worldspace = (M * vec4(VertexPosition_modelspace,1)).xyz;
	//Normal_worldspace = (M * vec4(VertexNormal_modelspace, 0)).xyz;
	// camera space! not world space
	Normal_worldspace = (V * M * vec4(VertexNormal_modelspace, 0)).xyz;
	
	UV = VertexUV;
}

