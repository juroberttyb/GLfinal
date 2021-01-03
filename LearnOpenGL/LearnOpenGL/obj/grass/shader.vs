#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 anormal;
layout (location = 2) in vec2 atexcoord;

out VS_OUT                                             
{                                                      
    vec3 N;                                            
    vec3 L;                                            
    vec3 V;     
    vec2 texcoord;
    vec4 FragPosLightSpace;
} vs_out;     

uniform vec3 light_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;
uniform mat4 lightSpaceMatrix;

void main()
{
    int id = gl_InstanceID;
    float x = mod(id, 9) - 2, z = id / 9 - 3, scale = 32;
    vec3 offset = vec3(scale*x, 0, scale*z);
    vec3 pos = aPos + offset;

    vec4 P = view * model * vec4(pos, 1.0);
    vs_out.N = mat3(transpose(inverse(view * model))) * anormal;
    vs_out.L = (view * vec4(light_pos, 1.0)).xyz - P.xyz;
    vs_out.V = -P.xyz;
    gl_Position = project * P;

    vs_out.texcoord = atexcoord;

    vs_out.FragPosLightSpace = lightSpaceMatrix * model * vec4(pos, 1.0);
} 