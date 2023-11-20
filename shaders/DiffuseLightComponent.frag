#version 410

struct Light {
    float ambient_factor;
    float specular_factor;
    float diffuse_factor;
    int shininess;
    float y;
    float x;
    float z;
};

in vec3 color_for_fragment_shader;
in vec3 normal_vector_for_fragment_shader;
in vec3 fragment_position;

uniform Light light_params;

out vec4 color_of_my_choice;

vec3 camera_position = vec3(0, 0, 1);
vec3 light_position = vec3(light_params.x, light_params.y, 1);

void main()
{
    vec3 view_direction = normalize(camera_position - fragment_position);
    vec3 light_direction = normalize(light_position - fragment_position);
    vec3 reflection_direction = reflect(-light_direction, normal_vector_for_fragment_shader);

    float ambient_factor = light_params.ambient_factor;
    float diffuse_factor = (max(dot(normal_vector_for_fragment_shader, light_direction), 0.0) * light_params.diffuse_factor);
    float specular_factor = pow(max(dot(view_direction, reflection_direction), 0.0), light_params.shininess) * light_params.specular_factor;

    color_of_my_choice = vec4((ambient_factor + diffuse_factor + specular_factor) * color_for_fragment_shader, 1.0);
    // color_of_my_choice = vec4(normal_vector_for_fragment_shader, 1.0);
}
