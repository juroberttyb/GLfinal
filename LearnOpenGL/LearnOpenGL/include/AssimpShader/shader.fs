#version 330 core

layout (location = 0) out vec4 color;    

in VS_OUT {
    vec3 N;                                            
    vec3 L;                                            
    vec3 V;  
    vec2 texcoord;
    mat3 TBN;
    vec4 FragPosLightSpace;
} fs_in;  

// uniform samplerCube skybox;

// Material properties                                                        
uniform sampler2D texture_diffuse1;       
uniform sampler2D texture_specular1;      
uniform sampler2D texture_normal1;
uniform sampler2D texture_h1;
uniform float specular_power = 200.0;  

uniform int NormalOn;

uniform sampler2D shadowMap;	

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}  

void main()
{
    // color = texture(texture_normal1, fs_in.texcoord);
    // color = texture(texture_diffuse1, fs_in.texcoord);
// /*
    vec3 N;

    if (NormalOn == 1)
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
    vec3 ambient = 0.1 * texture(texture_diffuse1, fs_in.texcoord).rgb;
    vec3 diffuse = max(dot(N, L), 0.0) * texture(texture_diffuse1, fs_in.texcoord).rgb;  
    vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * texture(texture_specular1, fs_in.texcoord).rgb;

    // color = vec4((ambient + diffuse + specular), 1.0);
    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);       
    vec3 lighting = ambient + (1.0 - shadow) * (diffuse + specular);

    color = vec4(lighting, 1.0);
// */
}