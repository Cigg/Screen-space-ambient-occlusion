#version 330 core

#define BLUR_SIZE 4

noperspective in vec2 UV;

out vec4 color;

uniform sampler2D SSAOTexture;
uniform float BlurSize;

void main() {
   vec2 texelSize = 1.0 / vec2(textureSize(SSAOTexture, 0));
   float result = 0.0;
   int b = int(BlurSize*0.5);
   for (int i = -b; i <= b; ++i) {
      for (int j = -b; j <= b; ++j) {
         vec2 offset = vec2(float(i), float(j)) * texelSize;
         result += texture(SSAOTexture, UV + offset).r;
      }
   }

   // result = texture(ssaoTexture, UV).r;
   result = result / ((BlurSize + 1.0)*(BlurSize + 1.0));

   color = vec4(vec3(result), 1.0);
}