#version 330

#define NUMBER_OF_LIGHTS 2

uniform vec3 light_pos[NUMBER_OF_LIGHTS];
uniform vec3 camera_pos;
uniform vec3 light_color[NUMBER_OF_LIGHTS];

out vec4 pixelColor; //Zmienna wyjsciowa fragment shadera. Zapisuje sie do niej ostateczny (prawie) kolor piksela

//Zmienne interpolowane
in vec3 pixel_position;
in vec3 pixel_normal;
in vec2 itexcoord;

uniform sampler2D texture_diffuse1;

vec3 add_lights(vec3 light_pos, vec3 light_color, vec3 normal, vec3 pixel_pos, vec3 view_direction)
{
	float ambient_s = 0.1; // sila oswietlenia od otoczenia
	vec3 ambient = ambient_s * light_color; //jak intesnywne oswieltenie

	vec3 light_direction = normalize(light_pos[0] - pixel_position);

	float diff = max(dot(normal, light_direction), 0);
	vec3 diffuse = diff * light_color * vec3(texture(texture_diffuse1, itexcoord));

	float specular_s = 0.213;
	
	//vec3 reflection = reflect(-light_direction, norm);
	//float spec = pow(max(dot(view_direction, reflection), 0), 16);

	vec3 half_way = normalize(light_direction + view_direction);
	float spec = pow(max(dot(view_direction, half_way), 0), 8);
	
	
	vec3 specular = specular_s * spec * light_color;

	vec3 result = (ambient + diffuse + specular) * vec3(1,1,1);
	
	return result;
}


void main(void) 
{
		vec3 norm = normalize(pixel_normal);
		vec3 view_direction = normalize(camera_pos - pixel_position);
		vec3 result;
		for(int i = 0; i < NUMBER_OF_LIGHTS; i++)
		{
			result += add_lights(light_pos[i], light_color[i], norm, pixel_position, view_direction);
		}
		pixelColor = vec4(result, 1.0);
}
