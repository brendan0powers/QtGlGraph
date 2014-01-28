#version 120
attribute float xAxis;
attribute float yAxis;
uniform float scaleFactor;
uniform float yOffset;
uniform mat4 transform;
uniform mat4 zoom;

void main(void)
{
    gl_Position = transform * zoom * vec4(xAxis, (yAxis * scaleFactor) + yOffset, 0.0, 1.0);
}
