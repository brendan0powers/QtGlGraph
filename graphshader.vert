#version 120
attribute float vertex;
uniform float scaleFactor;
uniform float yOffset;
uniform sampler2D texture;
uniform mat4 transform;

void main(void)
{
    float x = vertex;
    float y = (texture2D(texture, vec2(vertex,0.0)).r * scaleFactor) + yOffset;

    gl_Position = transform * vec4((x * 2.0) - 1.0, y, 0.0, 1.0);
}
