#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent; 

out VS_OUT                                             
{                                                      
    vec3 N;                                            
    vec3 L;                                            
    vec3 V;  
    vec2 texcoord;
    mat3 TBN;
    vec4 FragPosLightSpace;
} vs_out;     

uniform vec3 light_pos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 project;
uniform mat4 lightSpaceMatrix;

void main()
{
    mat3 normal_matrix = mat3(transpose(inverse(view * model)));

    vec4 P = view * model * vec4(aPos, 1.0);
    vs_out.N = normal_matrix * aNormal;
    vs_out.L = (view * vec4(light_pos, 1.0)).xyz - P.xyz;
    vs_out.V = -P.xyz;
    vs_out.texcoord = aTexCoords;
    gl_Position = project * P;

    vec3 T = normalize(normal_matrix * aTangent);
    vec3 B = normalize(normal_matrix * aBitangent);
    vec3 N = normalize(normal_matrix * aNormal);
    vs_out.TBN = mat3(T, B, N);

    vs_out.FragPosLightSpace = lightSpaceMatrix * model * vec4(aPos, 1.0);
} 