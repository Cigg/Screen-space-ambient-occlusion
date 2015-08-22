#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 VertexPosition_modelspace;

// Not interpolated texture coordinates
out vec2 UV;

void main(){
	gl_Position =  vec4(VertexPosition_modelspace,1);
	UV = (VertexPosition_modelspace.xy+vec2(1,1))/2.0;
}

