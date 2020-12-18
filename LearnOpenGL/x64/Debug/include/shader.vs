//  The main purpose of the vertex shader is to transform 3D coordinates into different 3D coordinates

// default basic types: int, float, double, uint and bool
// GLSL also features two container types, vectors and matrices



// A vector in GLSL is a 1,2,3 or 4 component container for any of the basic types just mentioned
// vecn: the default vector of n floats.
// bvecn: a vector of n booleans.
// ivecn: a vector of n integers.
// uvecn: a vector of n unsigned integers.
// dvecn: a vector of n double components.

// input and output
// When the types and the names are equal on both sides(shaders) OpenGL will link those variables together and then it is possible to send data between shaders (this is done when linking a program object)

#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
  
uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

void main()
{
    gl_Position = project * view * model * vec4(aPos, 1.0f);
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
} 