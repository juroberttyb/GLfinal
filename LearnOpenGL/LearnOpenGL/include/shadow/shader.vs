#version 330 core                

layout (location = 0) in vec3 position;                            
                          
uniform mat4 mvp;
uniform mat4 shadow_matrix;  

out vec4 shadow_coord;                                       

void main(void)                                            
{                                                                                              	
    shadow_coord = shadow_matrix * vec4(position, 1.0);	
    gl_Position = mvp * vec4(position, 1.0);	    
}	                                                      