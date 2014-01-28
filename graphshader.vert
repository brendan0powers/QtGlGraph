#version 120
attribute float xAxis;
attribute float yAxis;
uniform float scaleFactor;
uniform float yOffset;
uniform mat4 transform;

void main(void)
{
    gl_Position = transform * vec4(xAxis, (yAxis * scaleFactor) + yOffset, 0.0, 1.0);
}
