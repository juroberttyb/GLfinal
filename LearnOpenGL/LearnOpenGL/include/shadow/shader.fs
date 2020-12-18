#version 330 core                                                                         
                                                                                           
layout (location = 0) out vec4 color;	                                                    
uniform sampler2DShadow shadow_tex;	 
	                                                                                        
in VS_OUT	                                                                          
{	                                                                           
    vec4 shadow_coord;	                                                                    
    vec3 N;	                                                                 
    vec3 L;	                                                                    
    vec3 V;	                                                                  
} fs_in;

in vec3 Normal;
in vec3 Position;
	                                                     
// Material properties	                                                    
// uniform vec3 ambient = vec3(0.225, 0.2, 0.25);   	                                     
uniform vec3 diffuse_albedo = vec3(0.35);	                           
uniform vec3 specular_albedo = vec3(0.7);	                             
uniform float specular_power = 200.0;	                                           
uniform bool full_shading = true;	   

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform int mode;
	                                                                                
void main(void)	                                                             
{	                                                                               
    // Normalize the incoming N, L and V vectors	                              
    vec3 N = normalize(fs_in.N);	                                             
    vec3 L = normalize(fs_in.L);	                                               
    vec3 V = normalize(fs_in.V);	                                            
	vec3 H = normalize(L + V);	                                                            
    // Compute the diffuse and specular components for each fragment                    
    vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo;	                               
    vec3 specular = pow(max(dot(N, H), 0.0), specular_power) * specular_albedo;	      
    float shadow_factor = max(textureProj(shadow_tex, fs_in.shadow_coord), 0.2);   
    
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));

    if (mode == 0)
    {
        vec3 colorRGB = shadow_factor * (0.35 * texture(skybox, R).rgb + 0.65 * (diffuse + specular));
        color = vec4(colorRGB, 1.0);
    }
    else if (mode == 1)
    {
        if (shadow_factor > 0.5)
        {
            color =  vec4(1.0, 1.0, 1.0, 1.0);
        }
        else
        {
            color =  vec4(0.0, 0.0, 0.0, 1.0);
        }
    }
    else
    {
        color =  vec4(0.0, 0.0, 0.0, 1.0);
    }
}	                                                                                       