#version 330


out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela

in vec2 itexcoord;

uniform sampler2D texture_diffuse1;

uniform vec4 color=vec4(1,1,1,1);

void main(void) {
	pixelColor = vec4(vec3(texture(texture_diffuse1, itexcoord)), 1.0);
	//pixelColor = color;
}
