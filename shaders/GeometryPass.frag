#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;

// Ouput data
layout (location = 0) out vec3 WorldPosOut;
layout (location = 1) out vec3 DiffuseOut;
layout (location = 2) out vec3 NormalOut;
layout (location = 3) out vec3 TexCoordOut;

// Values that stay constant for the whole mesh.
uniform sampler2D DiffuseTexture;
uniform sampler2D DiffuseTextureMask;
uniform mat4 MV;
uniform vec3 LightPosition_worldspace;
uniform int HasTextureMask; // 1 = true, 0 = false

void main() {

	if(HasTextureMask == 1 && texture2D( DiffuseTextureMask, UV ).a < 0.01) {
		discard;
	}
	
	vec3 LightColor = vec3(1,1,1);

	vec3 n = normalize( Normal_cameraspace );
	
	WorldPosOut = Position_worldspace;
	DiffuseOut = texture2D( DiffuseTexture, UV ).rgb;
	NormalOut = n * 0.5 + 0.5;
	TexCoordOut = vec3(UV, 0.0);
}