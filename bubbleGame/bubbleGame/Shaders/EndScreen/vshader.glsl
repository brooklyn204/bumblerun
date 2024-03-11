#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texCords;

uniform vec2 translate;
uniform mat4 scale;

out vec2 vtex;

void main()
{
    vec4 scale_pos = scale * vec4(pos,1.0);
    gl_Position = vec4(scale_pos.x+translate.x,scale_pos.y+translate.y,scale_pos.zw);

    vtex = texCords;
}