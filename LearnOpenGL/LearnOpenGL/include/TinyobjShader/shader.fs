#version 330 core

layout (location = 0) out vec4 color;

in VS_OUT                                             
{                                                      
    vec3 N;                                            
    vec3 L;                                            
    vec3 V;  
    vec2 texcoord;
    vec4 FragPosLightSpace;
} fs_in;      

uniform samplerCube skybox;

// Material properties        
uniform vec3 diffuse_albedo = vec3(0.35);                            
uniform vec3 specular_albedo = vec3(0.7);                                     
uniform float specular_power = 200.0; 

uniform sampler2D shadowMap;	
// uniform sampler2D ssao;

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
    // Normalize the incoming N, L and V vectors                               
    vec3 N = normalize(fs_in.N);                                               
    vec3 L = normalize(fs_in.L);                                              
    vec3 V = normalize(fs_in.V);  
    vec3 H = normalize(L + V);                                                
                                                                                   
    // Compute the diffuse and specular components for each fragment  
    vec3 ambient = 0.3 * diffuse_albedo; // texture(ssao, fs_in.texcoord).r * 
    vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;  
    vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;

    vec3 I = -V;
    vec3 R = reflect(I, normalize(N));

    // color = 0.35 * vec4(texture(skybox, R).rgb, 1.0) + 0.65 * vec4((diffuse + specular), 1.0);

    float shadow = ShadowCalculation(fs_in.FragPosLightSpace);       
    vec3 lighting = ambient + (1.0 - shadow) * (0.35 * texture(skybox, R).rgb + 0.65 * (diffuse + specular));

    color = vec4(lighting, 1.0);
}

