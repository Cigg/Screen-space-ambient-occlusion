#version 330 core

#define KERNEL_SIZE 32
#define KERNEL_RADIUS 4

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

uniform vec3 SSAOKernel[KERNEL_SIZE];
uniform vec2 NoiseScale;
uniform sampler2D NoiseTexture;

float rand(vec2 co) {
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec4 getViewSpacePosition(vec2 uv) {
    float x = uv.s * 2.0 - 1.0;
    float y = uv.t * 2.0 - 1.0;
    float z = texture(DepthTexture, uv).r;

    vec4 pos = InverseProjectionMatrix * vec4(x, y, z, 1.0);
    pos /= pos.w;

    return pos;
}

void main() {
    //vec3 worldPos = texture2D( WorldPositionTexture, UV ).rgb;
    vec3 diffuseColor = texture2D( ColorTexture, UV ).rgb;
    vec3 normal = normalize(texture( NormalTexture, UV ).xyz * 2.0 - 1.0);
    //vec3 normal = texture( NormalTexture, UV ).xyz;

    vec3 lightDirection = normalize(LightDirection_worldspace);
    float cosTheta = clamp(dot(normal, lightDirection), 0, 1);

    vec3 ambientColor = vec3(0.1,0.1,0.1) * diffuseColor;
    // vec3 specularColor = vec3(0.3,0.3,0.3);
    // vec3 E = normalize(EyePosition_worldspace - worldPos);
    // vec3 R = reflect(-l,n);
    // float cosAlpha = clamp( dot( E,R ), 0,1 );

    // ----------------- Calculate occlusion factor ---------------------
    // Caclulate view space position and normal
    vec4 viewSpacePosition = getViewSpacePosition(UV);
    vec3 randomVec = normalize(texture(NoiseTexture, UV * NoiseScale).xyz);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 transformationMatrix = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < KERNEL_SIZE; i++) {
        // Get sample position
        vec4 samplePoint = vec4(transformationMatrix * SSAOKernel[i], 0.0);
        samplePoint = samplePoint * KERNEL_RADIUS + viewSpacePosition;
        float z = samplePoint.z;

        // Project sample position;
        samplePoint = ProjectionMatrix * samplePoint;
        samplePoint /= samplePoint.w;
        vec2 sampleTexCoord = samplePoint.xy * 0.5 + 0.5;

        // Get sample depth
        float sampleDepth = texture(DepthTexture, sampleTexCoord).r;
        // float linearDepth = sampleDepth * 2.0 - 1.0;
        // float A = ProjectionMatrix[2].z;
        // float B = ProjectionMatrix[3].z;
        // float zNear = - B / (1.0 - A);
        // float zFar  =   B / (1.0 + A);
        // linearDepth = zNear * zFar / (zFar + zNear - linearDepth * (zFar - zNear));

        float delta = samplePoint.z - sampleDepth;
        
        float rangeCheck = 1.0;//abs(viewSpacePosition.z - linearDepth) < KERNEL_RADIUS ? 1.0 : 0.0;
        occlusion += (delta > 0.00001 ? 1.0 : 0.0) * rangeCheck;
    }

    occlusion = 1.0 - (occlusion / float(KERNEL_SIZE));

    // // color = vec4(ambientColor + diffuseColor * cosTheta + specularColor * pow(cosAlpha,5), 1.0);
    // // color = vec4(ambientColor + diffuseColor * cosTheta, 1.0);
    color = vec4(vec3(occlusion), 1.0);

    // float depth = texture(DepthTexture, UV).x * 2.0 - 1.0;
    // depth = depth > 0.9 ? (depth - 0.9)/0.1 : 0;
    // color = vec4(vec3(depth), 1.0);
}
