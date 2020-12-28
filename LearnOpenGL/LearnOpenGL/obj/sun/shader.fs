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
    vec3 temp = texture(texture_diffuse1, texcoord).rgb;

    if (temp.r != 0)
    {
        temp = vec3(4.0);
    }
    else
    {
        temp = vec3(1.0);
    }
    
    color = vec4(temp, 1.0);
}