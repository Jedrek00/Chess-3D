#version 330

uniform float Time;

in vec3 PixelPos;
in vec3 Norms;
in vec2 Tex;

out vec4 PixelColor;

vec3 rainbow(float level)
{
	float R = float(level <= 2.0) + float(level > 4) * 0.5;
	float G = max(1.0 - abs(level - 2.0) * 0.5, 0);
	float B = (1.0 - (level - 4.0) * 0.5) * float(level >= 4);
	return vec3(R,G,B);
}

vec3 smoothr(float x)
{
	float level1 = floor(x*6.0);
	float level2 = min(6.0, floor(x*6.0)+1);
	vec3 a = rainbow(level1);
	vec3 b = rainbow(level2);
	return mix(a, b, fract(x*6.0));
}

void main()
{
	PixelColor = vec4(smoothr(0.7 * Time), 1.0);
}