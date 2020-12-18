#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 atexcoord;
layout (location = 2) in vec3 anormal;

out VS_OUT                                             
{                                                      
    vec3 N;                                            
    vec3 L;                                            
    vec3 V;       
} vs_out;     

uniform vec3 cameraPos;
uniform vec3 light_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

void main()
{
    vec4 P = view * model * vec4(aPos, 1.0);
    vs_out.N = mat3(transpose(inverse(view * model))) * anormal;
    vs_out.L = (view * vec4(light_pos, 1.0)).xyz - P.xyz;
    vs_out.V = -P.xyz;
    gl_Position = project * P;
} 