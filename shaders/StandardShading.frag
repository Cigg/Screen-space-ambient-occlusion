#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;
in vec3 Position_worldspace;
in vec3 Normal_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;

// Ouput data
out vec4 color;

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
	float LightPower = 50.0f;
	
	vec3 MaterialDiffuseColor = texture2D( DiffuseTexture, UV ).rgb;

	vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
	vec3 MaterialSpecularColor = vec3(0.3,0.3,0.3);

	vec3 n = normalize( Normal_cameraspace );
	vec3 l = normalize( LightDirection_cameraspace );
	float cosTheta = clamp( dot( n,l ), 0,1 );
	
	vec3 E = normalize(EyeDirection_cameraspace);
	vec3 R = reflect(-l,n);
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
	color = vec4(
		MaterialAmbientColor +
		MaterialDiffuseColor * LightColor * cosTheta +
		MaterialSpecularColor * LightColor * pow(cosAlpha,5), 1.0);

}