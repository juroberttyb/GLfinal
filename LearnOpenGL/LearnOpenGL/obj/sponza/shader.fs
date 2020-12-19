#version 330 core

layout (location = 0) out vec4 color;    

in VS_OUT {
    vec3 N;                                            
    vec3 L;                                            
    vec3 V;  
    vec2 texcoord;
    mat3 TBN;
} fs_in;  

// uniform samplerCube skybox;

// Material properties                                                        
uniform sampler2D texture_diffuse1;       
uniform sampler2D texture_specular1;      
uniform sampler2D texture_normal1;
uniform sampler2D texture_h1;
uniform float specular_power = 200.0;  

uniform int on;

void main()
{
    vec3 N;

    if (on == 1)
    {
        // obtain normal from normal map in range [0,1]
        N = texture(texture_normal1, fs_in.texcoord).rgb;
        // transform normal vector to range [-1,1]
        N = N * 2.0 - 1.0;
        N = normalize(fs_in.TBN * N);
    }
    else
    {
        N = normalize(fs_in.N);
    }

    // Normalize the incoming N, L and V vectors                               
    // vec3 N = normalize(fs_in.N);                                               
    vec3 L = normalize(fs_in.L);                                              
    vec3 V = normalize(fs_in.V);  
    vec3 H = normalize(L + V);                                                
                                                                                   
    // Compute the diffuse and specular components for each fragment    
    vec3 ambient = 0.1 * texture(texture_h1, fs_in.texcoord).rgb;
    vec3 diffuse = max(dot(N, L), 0.0) * texture(texture_diffuse1, fs_in.texcoord).rgb;  
    vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * texture(texture_specular1, fs_in.texcoord).rgb;

    vec3 I = -V;
    vec3 R = reflect(I, normalize(N));

    // color = 0.35 * vec4(texture(skybox, R).rgb, 1.0) + 0.65 * vec4((diffuse + specular), 1.0);
    color = vec4((ambient + diffuse + specular), 1.0);
}

