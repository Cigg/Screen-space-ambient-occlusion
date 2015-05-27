#version 330 core

noperspective in vec2 UV;

out vec4 color;

// uniform sampler2D WorldPositionTexture;
uniform sampler2D ColorTexture;
uniform sampler2D NormalTexture;
uniform sampler2D DepthTexture;
uniform sampler2D SSAOTexture;

uniform vec3 LightDirection_worldspace;
uniform vec3 EyePosition_worldspace;
uniform mat4 InverseViewMatrix;

void main() {
    vec3 normal = normalize(texture( NormalTexture, UV ).xyz * 2.0 - 1.0);
    vec3 normalWorldSpace = (InverseViewMatrix * vec4(normal, 0.0)).xyz;

    vec3 diffuseColor = texture2D( ColorTexture, UV ).rgb;
    vec3 ambientColor = vec3(0.1,0.1,0.1) * diffuseColor;

    vec3 lightDirection = normalize(LightDirection_worldspace);
    float cosTheta = clamp(dot(normalWorldSpace, lightDirection), 0, 1);

    float occlusion = texture(SSAOTexture, UV).r;

    // color = vec4(ambientColor + diffuseColor * cosTheta + specularColor * pow(cosAlpha,5), 1.0);
    color = vec4(occlusion*(ambientColor + diffuseColor * cosTheta), 1.0);
    // color = vec4(vec3(occlusion), 1.0);
}
