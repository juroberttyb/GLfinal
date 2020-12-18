#version 330 core
out vec4 FragColor;
  
in vec2 TexCoords;

uniform sampler2D combine;
uniform sampler2D shad;
uniform sampler2D noshad;

uniform int mode;

void main()
{ 
    if (mode == 0)
    {
        FragColor = texture(combine, TexCoords) + texture(shad, TexCoords) - texture(noshad, TexCoords);
    }
    else if (mode == 1)
    {
        FragColor = texture(combine, TexCoords);
    }
    else if (mode == 2)
    {
        FragColor = texture(shad, TexCoords);
    }
    else
    {
        FragColor = texture(noshad, TexCoords);
    }
}