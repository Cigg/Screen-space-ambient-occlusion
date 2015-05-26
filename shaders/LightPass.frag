#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D WorldPositionTexture;
uniform sampler2D ColorTexture;
uniform sampler2D NormalTexture;
uniform sampler2D DepthTexture;

uniform vec3 LightDirection_worldspace;
uniform vec3 EyePosition_worldspace;
uniform mat4 ProjectionMatrix;
uniform mat4 InverseProjectionMatrix;

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 getViewSpacePosition(vec2 uv) {
    float x = uv.s * 2.0 - 1.0;
    float y = uv.t * 2.0 - 1.0;
    float z = texture(DepthTexture, uv).r * 2.0 - 1.0;

    vec4 pos = InverseProjectionMatrix * vec4(x, y, z, 1.0);
    pos /= pos.w;

    return pos.xyz;
}

void main() {
    //vec3 worldPos = texture2D( WorldPositionTexture, UV ).rgb;
    vec3 diffuseColor = texture2D( ColorTexture, UV ).rgb;
    vec3 normal = texture( NormalTexture, UV ).xyz;
    vec3 lightDirection = normalize(LightDirection_worldspace);
    float cosTheta = clamp(dot(normal, lightDirection), 0, 1);

    vec3 ambientColor = vec3(0.1,0.1,0.1) * diffuseColor;
    // vec3 specularColor = vec3(0.3,0.3,0.3);
    // vec3 E = normalize(EyePosition_worldspace - worldPos);
    // vec3 R = reflect(-l,n);
    // float cosAlpha = clamp( dot( E,R ), 0,1 );

    // ----------------- Calculate occlusion factor ---------------------
    // Caclulate view space position and normal
    vec3 viewSpacePosition = getViewSpacePosition(UV);
    vec3 randomVec = normalize(vec3(rand(UV), rand(UV*0.9), rand(UV*1.1)));
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 transformationMatrix = mat3(tangent, bitangent, normal);

    // float occlusion = 0.0;
    // int kernelSize = 64;
    // float radius = 4.0;
    // for(int i = 0; i < kernelSize; ++i) {
    //     // Get sample position
    //     vec3 sample = transformationMatrix * vec3(rand(UV + 0.003*i), rand(UV + 0.01095*i), rand(UV + 0.251254*i));
    //     sample = sample * radius + viewSpacePosition;

    //     // Project sample position;
    //     vec4 offset = vec4(sample, 1.0);
    //     offset = ProjectionMatrix * offset;
    //     offset.xy /= offset.w;
    //     offset.xy = offset.xy * 0.5 + 0.5;

    //     // Get sample depth
    //     float sampleDepth = texture(DepthTexture, offset.xy).r;

    //     // // Range check and accumulate
    //     // float rangeCheck = abs(viewSpacePosition.z - sampleDepth) < radius ? 1.0 : 0.0;
    //     occlusion += (sampleDepth <= sample.z ? 1.0 : 0.0);
    // }

    // occlusion = 1.0 - occlusion / (float(kernelSize) - 1.0);

    //color = vec4(ambientColor + diffuseColor * cosTheta + specularColor * pow(cosAlpha,5), 1.0);
    //color = vec4(ambientColor + diffuseColor * cosTheta, 1.0);
    //color = vec4(vec3(occlusion), 1.0);

    float depth = texture(DepthTexture, UV).x;
    depth = depth > 0.9 ? (depth - 0.9)/0.1 : 0;
    color = vec4(vec3(depth), 1.0);
}
