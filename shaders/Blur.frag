#version 330 core

out vec4 color;

uniform sampler2D SSAOTexture;
uniform float BlurSize;
uniform vec2 ScreenSize;

void main() {
   vec2 uv = gl_FragCoord.xy/ScreenSize;
   vec2 texelSize = 1.0 / vec2(textureSize(SSAOTexture, 0));
   float result = 0.0;
   vec2 hlim = vec2(float(-BlurSize) * 0.5);
   for (int i = 0; i < BlurSize; ++i) {
      for (int j = 0; j < BlurSize; ++j) {
         vec2 offset = (hlim + vec2(float(i), float(j))) * texelSize;
         result += texture(SSAOTexture, uv + offset).r;
      }
   }

   // result = texture(ssaoTexture, UV).r;
   result = result / float(BlurSize*BlurSize);

   color = vec4(vec3(result), 1.0);
}