#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;

uniform float time;
uniform mat4 proj_matrix;
flat out vec4 starColor;

void main(void)
{
	vec4 starPosition = vec4(position.x, position.y + time, position.z, 1.0);
	starPosition.y = int(starPosition.y * 10000) % 10000 / 10000.0;
	float starSize = 8.0 * starPosition.y * starPosition.y;
	starColor = vec4(color * smoothstep(1.0, 7.0, starSize), 1.0);
	starPosition.y = 2500.0 - (2500.0 * starPosition.y) - 100.0;
	gl_Position = proj_matrix * starPosition;
	gl_PointSize = starSize;
}