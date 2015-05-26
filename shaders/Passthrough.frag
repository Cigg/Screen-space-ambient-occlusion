#version 330 core

in vec2 UV;

out vec4 color;

uniform sampler2D QuadTexture;
uniform sampler2D DepthTexture;
uniform sampler2D NormalTexture;

void main(){
	color = vec4(texture2D( DepthTexture, UV ).rgb, 1.0);
}
