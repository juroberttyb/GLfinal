#version 330 core
out vec4 FragColor;
  
uniform sampler2D scene;

in vec2 TexCoords;

void main()
{ 
    float AmbientOcclusion = texture(scene, TexCoords).r;
    FragColor = vec4(AmbientOcclusion, AmbientOcclusion, AmbientOcclusion, 1.0);
}