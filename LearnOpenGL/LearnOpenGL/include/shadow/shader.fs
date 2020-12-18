#version 330 core                                                                         
                                                                                           
layout (location = 0) out vec4 color;	                                                    
uniform sampler2DShadow shadow_tex;	 
	                    
uniform int shapeonly;
in vec4 shadow_coord;	                                                                    
	                                                                                
void main(void)	                                                             
{	                                                                                     
    if (shapeonly == 0)
    {
        float shadow_factor = max(textureProj(shadow_tex, shadow_coord), 0.2);  

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
        color =  vec4(1.0, 1.0, 1.0, 1.0);
    }
}	                                                                                       