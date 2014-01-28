#version 120
#extension GL_EXT_gpu_shader4 : require

attribute vec2 vertex;
uniform mat4 transform;

void main(void)
{
    gl_Position = transform * vec4(vertex, 0.0, 1.0);
}
