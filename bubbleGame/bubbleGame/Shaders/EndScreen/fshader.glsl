#version 330 core

in vec2 vtex;

uniform sampler2D text;

out vec4 outColor;

void main()
{
    outColor = texture(text,vtex);
}