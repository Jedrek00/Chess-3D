#version 330

out vec4 pixelColor;

in vec3 PixelPos;
in vec3 Norms;
in vec2 Tex;

in vec4 ic;

void main(void)
{
	pixelColor=ic;
}