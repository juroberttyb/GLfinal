#version 330 core

layout (location = 0) out vec4 color;

in vec2 texcoord;

in VS_OUT                                             
{                                                      
    vec3 N;                                            
    vec3 L;                                            
    vec3 V;  
} fs_in;      

uniform samplerCube skybox;

// Material properties                                                        
uniform vec3 diffuse_albedo = vec3(0.35);                            
uniform vec3 specular_albedo = vec3(0.7);                                     
uniform float specular_power = 200.0;  

uniform sampler2D texture_diffuse0;

uniform int shapeonly = 1;

void main()
{
    // Normalize the incoming N, L and V vectors                               
    vec3 N = normalize(fs_in.N);                                               
    vec3 L = normalize(fs_in.L);                                              
    vec3 V = normalize(fs_in.V);  
    vec3 H = normalize(L + V);                                                
                                                                                   
    // Compute the diffuse and specular components for each fragment           
    vec3 diffuse = max(dot(N, L), 0.0) * (texture(texture_diffuse0, texcoord)).xyz;  
    vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;

    vec3 I = -V;
    vec3 R = reflect(I, normalize(N));

    if (shapeonly == 1)
    {
        color = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        color = 0.35 * vec4(texture(skybox, R).rgb, 1.0) + 0.65 * vec4((diffuse + specular), 1.0);
    }
}

