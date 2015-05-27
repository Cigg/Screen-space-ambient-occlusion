#version 330 core

#define MAX_KERNEL_SIZE 128

noperspective in vec2 UV;

out vec4 color;

uniform sampler2D NormalTexture;
uniform sampler2D DepthTexture;

uniform mat4 ProjectionMatrix;
uniform mat4 InverseProjectionMatrix;

uniform vec3 SSAOKernel[MAX_KERNEL_SIZE];
uniform vec2 NoiseScale;
uniform sampler2D NoiseTexture;
uniform int KernelSize;
uniform int KernelRadius;
uniform float IsTurnedOn;

vec4 getViewSpacePosition(vec2 uv) {
    float x = uv.s * 2.0 - 1.0;
    float y = uv.t * 2.0 - 1.0;
    float z = texture(DepthTexture, uv).r;

    vec4 pos = InverseProjectionMatrix * vec4(x, y, z, 1.0);
    pos /= pos.w;

    return pos;
}

void main() {
    vec3 normal = normalize(texture( NormalTexture, UV ).xyz * 2.0 - 1.0);

    // Caclulate view space position and normal
    vec4 viewSpacePosition = getViewSpacePosition(UV);
    vec3 randomVec = normalize(texture(NoiseTexture, UV * NoiseScale).xyz);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 transformationMatrix = mat3(tangent, bitangent, normal);

    float occlusion = 1.0;
    if(IsTurnedOn > 0.5) {
        occlusion = 0.0;
        for(int i = 0; i < KernelSize; i++) {
            // Get sample position
            vec4 samplePoint = vec4(transformationMatrix * SSAOKernel[i], 0.0);
            samplePoint = samplePoint * KernelRadius + viewSpacePosition;
            float z = samplePoint.z;

            // Project sample position;
            samplePoint = ProjectionMatrix * samplePoint;
            samplePoint /= samplePoint.w;
            vec2 sampleTexCoord = samplePoint.xy * 0.5 + 0.5;

            // Get sample depth
            float sampleDepth = texture(DepthTexture, sampleTexCoord).r;
            float delta = samplePoint.z - sampleDepth;

            // Get real depth and check if sample is within the kernel radius
            float linearDepth = getViewSpacePosition(sampleTexCoord).z;
            float rangeCheck = abs(viewSpacePosition.z - linearDepth) < KernelRadius ? 1.0 : 0.0;

            // Contribute to occlusion if within radius and larger than a small number to prevent crazy artifactsZ
            occlusion += (delta > 0.00005 ? 1.0 : 0.0) * rangeCheck;
        }

        occlusion = 1.0 - (occlusion / float(KernelSize));
    }

    color = vec4(vec3(occlusion), 1.0);
}
