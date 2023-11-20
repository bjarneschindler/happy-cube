#version 410

in vec3 color;
in vec3 position;
in vec3 normal_vector;

uniform mat4 matrix;

out vec3 color_for_fragment_shader;
out vec3 normal_vector_for_fragment_shader;
out vec3 fragment_position;

void main()
{
    gl_Position = matrix * vec4(position, 1.0);
    fragment_position = (matrix * vec4(position, 1.0)).xyz;
    color_for_fragment_shader = color;
    normal_vector_for_fragment_shader = (matrix * vec4(normal_vector, 1.0)).xyz;
    // normal_vector_for_fragment_shader = normal_vector;
}
