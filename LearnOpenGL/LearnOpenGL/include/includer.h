#define _CRT_SECURE_NO_WARNINGS
#define TINYOBJLOADER_IMPLEMENTATION

#include <glad.h> // Be sure to include GLAD before GLFW. The include file for GLAD includes the required OpenGL headers behind the scenes (like GL/gl.h) so be sure to include GLAD before other header files that require OpenGL (like GLFW).
// GLAD manages function pointers in graphic card for OpenGL
#include <glfw3.h> // for creating window
#include <iostream>
#include <fbxsdk.h>
#include <vector>
#include <string>

#include "transform.h"
#include "Window.h"
#include "pipeline.h"
#include "tiny_obj_loader.h"
//#include "fbxloader.h"

#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <IL/il.h>

using namespace std;
using namespace glm;

typedef struct _fbx_handles
{
	_fbx_handles()
	{
		lSdkManager = NULL;
		lScene = NULL;
	}

	FbxManager* lSdkManager;
	FbxScene* lScene;
	FbxArray<FbxString*> lAnimStackNameArray;
} fbx_handles;

void GetFbxAnimation(fbx_handles &handles, std::vector<tinyobj::attrib_t> &attribs, std::vector<tinyobj::shape_t> &shapes, float frame);
bool LoadFbx(fbx_handles &handles, std::vector<tinyobj::attrib_t> &attribs, std::vector<tinyobj::shape_t> &shapes, std::vector<tinyobj::material_t> &materials, std::string err, const char* fbxFile);
void ReleaseFbx(fbx_handles &handles);