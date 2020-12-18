#define _CRT_SECURE_NO_WARNINGS
#define TINYOBJLOADER_IMPLEMENTATION

#include <glad.h> // Be sure to include GLAD before GLFW. The include file for GLAD includes the required OpenGL headers behind the scenes (like GL/gl.h) so be sure to include GLAD before other header files that require OpenGL (like GLFW).
// GLAD manages function pointers in graphic card for OpenGL
#include <glfw3.h> // for creating window
#include <iostream>

#include "transform.h"
#include "Window.h"
#include "pipeline.h"
#include "tiny_obj_loader.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>

using namespace std;
using namespace glm;
