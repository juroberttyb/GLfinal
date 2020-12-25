#version 330 core

uniform mat4 mvp;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texcoord;

out vec2 TexCoords;

uniform sampler2D hmap;

void main()
{
	float v = (texture2DLod(hmap, texcoord, 0.0).r - 0.5) * 64; // texture2D(hmap, vec2(position.x, position.z)).r; // texture2DProjLod(hmap, vec3(position.x, position.z, 0.5), 1.0).a;
	gl_Position = mvp * vec4(position.x, v, position.y, 1.0);
	TexCoords = texcoord;
}