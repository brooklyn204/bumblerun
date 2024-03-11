#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCords;
layout (location = 2) in vec3 norm;

uniform mat4 transform_matrix;
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 normal_matrix;
uniform mat4 projection_matrix;

out vec2 vtex;
out vec3 vnorm;
out vec3 fpos;


void main()
{
    gl_Position = projection_matrix * view_matrix * model_matrix * transform_matrix * vec4(pos,1.0);

    vtex = texCords;
    fpos = vec3(model_matrix * vec4(pos,1.0));

    // Translate normal vector to world coordinates, to be consistent with fpos coordinates in the fragment shader
    vnorm = mat3(normal_matrix) * norm;
}