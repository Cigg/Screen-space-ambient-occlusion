#version 330 core

in vec2 UV;

out vec4 color;

// uniform sampler2D WorldPositionTexture;
uniform sampler2D ColorTexture;
uniform sampler2D NormalTexture;
uniform sampler2D DepthTexture;
uniform sampler2D SSAOTexture;

uniform vec3 LightDirection_worldspace;
uniform vec3 EyePosition_worldspace;
uniform mat4 InverseViewMatrix;
uniform float UseTexture;
uniform vec2 ScreenSize;

void main() {
	vec2 uv = gl_FragCoord.xy/ScreenSize;
    vec3 normal = normalize(texture( NormalTexture, uv ).xyz * 2.0 - 1.0);
    vec3 normalWorldSpace = (InverseViewMatrix * vec4(normal, 0.0)).xyz;

    vec3 diffuseColor = texture2D( ColorTexture, uv ).rgb;
    vec3 ambientColor = vec3(0.15,0.15,0.15) * diffuseColor;

    vec3 lightDirection = normalize(LightDirection_worldspace);
    float cosTheta = clamp(dot(normalWorldSpace, lightDirection), 0, 1);

    float occlusion = texture(SSAOTexture, uv).r;
    occlusion = occlusion*1.3 - 0.3;
    // color = vec4(ambientColor + diffuseColor * cosTheta + specularColor * pow(cosAlpha,5), 1.0);
    color = UseTexture > 0.5 ?  vec4(occlusion*(ambientColor + diffuseColor * cosTheta), 1.0) :
                                vec4(vec3(occlusion), 1.0);
    // color = vec4(vec3(occlusion), 1.0);
}
