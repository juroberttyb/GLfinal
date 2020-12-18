// The main purpose of the fragment shader is to calculate the final color of a pixel
// A fragment in OpenGL is all the data required for OpenGL to render a single pixel

// Fragment interpolation is applied to all the fragment shader's input attributes.

// in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  

// GLSL has a built-in data-type for texture objects called a sampler that takes as a postfix the texture type we want 
// e.g. sampler1D, sampler3D or in our case sampler2D. We can then add a texture to the fragment shader by simply declaring a uniform sampler2D that we later assign our texture to



// texture() 
// takes as its first argument a texture sampler and as its second argument the corresponding texture coordinates
// The texture function then samples the corresponding color value using the texture parameters we set earlier. 
// The output of this fragment shader is then the (filtered) color of the texture at the (interpolated) texture coordinate

// bind the texture before calling glDrawElements and it will then automatically assign the texture to the fragment shader's sampler

// GLSL's built-in mix() function takes two values as input and linearly interpolates between them based on its third argument
// If the third value is 0.0 it returns the first input; if it's 1.0 it returns the second input value. A value of 0.2 will return 80% of the first input color and 20% of the second input color, resulting in a mixture of both our textures.

#version 330 core
out vec4 FragColor;
  
in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture0;
uniform sampler2D texture1;

void main()
{
    FragColor = mix(texture(texture0, TexCoord), texture(texture1, TexCoord), 0.2);
}

