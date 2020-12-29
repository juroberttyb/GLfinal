#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
uniform sampler2D tex;

void main()
{
	FragColor = 4 * texture(tex, TexCoords); // vec4(0.0, 1.0, 0.0, 1.0); // texture(hmap, TexCoords);
}