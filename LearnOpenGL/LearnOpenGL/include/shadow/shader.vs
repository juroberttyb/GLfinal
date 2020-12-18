#version 330 core                

uniform mat4 model;  
uniform mat4 mv_matrix;                                   
uniform mat4 proj_matrix;                                  
uniform mat4 shadow_matrix;                    

layout (location = 0) in vec3 position;                    
layout (location = 1) in vec2 texCoord;                   
layout (location = 2) in vec3 normal;         

out VS_OUT                                                 
{                                                          
    vec4 shadow_coord;                                     
    vec3 N;                                                
    vec3 L;                                                
    vec3 V;                                                
} vs_out; 

out vec3 Normal;
out vec3 Position;

// Position of light                                       
uniform vec3 light_pos = vec3(-31.75, 26.05, -97.72);      

void main(void)                                            
{                                                         
    // Calculate view-space coordinate                     
    vec4 P = mv_matrix * vec4(position, 1.0);	
    
    // Calculate normal in view-space	                    
    vs_out.N = mat3(mv_matrix) * normal; // transpose(inverse(mat3(mv_matrix))) * normal;
    
    // Calculate light vector	                            
    vs_out.L = light_pos - P.xyz;	
    
    // Calculate view vector	                            
    vs_out.V = -P.xyz;	        
    
    // Light-space coordinates                          	
    vs_out.shadow_coord = shadow_matrix * vec4(position, 1.0);	

    // Calculate the clip-space position of each vertex	
    gl_Position = proj_matrix * P;	    
    
    Normal = mat3(transpose(inverse(model))) * normal;
    Position = vec3(model * vec4(position, 1.0));
}	                                                      