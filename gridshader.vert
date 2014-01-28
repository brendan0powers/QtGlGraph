#version 120

attribute vec2 vertex;
uniform mat4 transform;

void main(void)
{
    gl_Position = transform * vec4(vertex, 0.0, 1.0);
}
