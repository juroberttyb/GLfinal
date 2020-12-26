#version 330 core

layout (location = 0) out vec4 color;    

in vec2 texcoord;

// uniform samplerCube skybox;

// Material properties                                                        
uniform sampler2D texture_diffuse1;       
uniform sampler2D texture_specular1;      
uniform sampler2D texture_normal1;
uniform sampler2D texture_h1;

void main()
{
    color = texture(texture_diffuse1, texcoord);
}