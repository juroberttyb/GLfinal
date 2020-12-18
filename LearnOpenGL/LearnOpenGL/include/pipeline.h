#pragma once

#include <glad.h> // Be sure to include GLAD before GLFW. The include file for GLAD includes the required OpenGL headers behind the scenes (like GL/gl.h) so be sure to include GLAD before other header files that require OpenGL (like GLFW).
// GLAD manages function pointers in graphic card for OpenGL
#include "tiny_obj_loader.h"
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <Importer.hpp>
#include <scene.h>
#include <postprocess.h>

#include <string>
#include <vector>
#include <iostream>

using namespace std;

class TinyOjectLoader
{
public:
    void load(const char* path) // const char* name
    {
        tinyobj::attrib_t attrib;
        vector<tinyobj::shape_t> shapes;
        vector<tinyobj::material_t> materials;

        string warn;
        string err;
        bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path);
        if (!warn.empty()) {
            cout << warn << endl;
        }
        if (!err.empty()) {
            cout << err << endl;
        }
        if (!ret) {
            exit(1);
        }

        cout << attrib.vertices.size() << endl;
        cout << attrib.texcoords.size() << endl;
        cout << attrib.normals.size() << endl;

        int VertexCount = 0;
        vector<float> vertices, texcoords, normals;  // if OBJ preserves vertex order, you can use element array buffer for memory efficiency
        for (int s = 0; s < shapes.size(); ++s) {  // for 'ladybug.obj', there is only one object
            int index_offset = 0;
            for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); ++f) {
                int fv = shapes[s].mesh.num_face_vertices[f];
                for (int v = 0; v < fv; ++v) {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
                    vertices.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
                    texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
                    texcoords.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
                    normals.push_back(attrib.normals[3 * idx.normal_index + 0]);
                    normals.push_back(attrib.normals[3 * idx.normal_index + 1]);
                    normals.push_back(attrib.normals[3 * idx.normal_index + 2]);
                }
                index_offset += fv;
                vnum += fv;
            }
        }

        glGenVertexArrays(1, &this->vao);
        glBindVertexArray(this->vao);

        glGenBuffers(1, &this->vbo);
        glBindBuffer(GL_ARRAY_BUFFER, this->vbo);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float) + normals.size() * sizeof(float), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
        glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), texcoords.size() * sizeof(float), texcoords.data());
        glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float) + texcoords.size() * sizeof(float), normals.size() * sizeof(float), normals.data());

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(float) + texcoords.size() * sizeof(float)));
        glEnableVertexAttribArray(2);

        shapes.clear();
        shapes.shrink_to_fit();
        materials.clear();
        materials.shrink_to_fit();
        vertices.clear();
        vertices.shrink_to_fit();
        texcoords.clear();
        texcoords.shrink_to_fit();
        normals.clear();
        normals.shrink_to_fit();

        cout << vnum << " vertices loaded." << endl;

        /*
        texture_data tdata = loadImg("ladybug_diff.png");

        glGenTextures(1, &m_shape.m_texture);
        glBindTexture(GL_TEXTURE_2D, m_shape.m_texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tdata.w, tdata.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tdata.data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        delete tdata.data;
        */

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    GLuint vao = 0;			// vertex array object
    GLuint vbo = 0;			// vertex buffer object

    int vnum = 0;
};
class ShaderProgram
{
public:
    ShaderProgram(const char* vertexPath, const char* fragmentPath)
    {
        vertex_shader = new Shader(vertexPath, GL_VERTEX_SHADER);
        fragment_shader = new Shader(fragmentPath, GL_FRAGMENT_SHADER);

        // shader Program
        CreateProgram(vertex_shader->id, fragment_shader->id);
        clean(vertex_shader->id, fragment_shader->id);

        delete(vertex_shader);
        delete(fragment_shader);
    }

    void TextureFromFile(const string filename, bool flip = false)
    {
        cout << filename << endl;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int w, h, nrComponents;
        stbi_set_flip_vertically_on_load(flip);
        unsigned char* data = stbi_load(filename.c_str(), &w, &h, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;
            else
                cout << "format not initialized" << endl;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            cout << "Texture failed to load at path: " << filename << endl;
            stbi_image_free(data);
        }

        glBindTexture(GL_TEXTURE_2D, 0);
        TextureList.push_back(textureID);
    }

    void BindTexture(const char* UniformName, unsigned int TextureUnit, unsigned int texture)
    {
        unsigned int SamplerID = TextureUnit - GL_TEXTURE0;

        // tell OpenGL to which texture unit each shader sampler belongs to by setting each sampler using glUniform1i
        glUseProgram(id); // don't forget to activate the shader before setting uniforms!  
        glUniform1i(glGetUniformLocation(id, UniformName), SamplerID); // Using glUniform1i we can actually assign a location value to the texture sampler so we can set multiple textures at once
        glUseProgram(0);

        glActiveTexture(TextureUnit);
        glBindTexture(GL_TEXTURE_2D, texture);
    }

    void SetUniformMat(const char* name, glm::mat4 value)
    {
        glUseProgram(id); // use program so that we can change uniform value
        unsigned int location = glGetUniformLocation(id, name);
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
        glUseProgram(0);
    }

    void SetUniformInt(const char* name, int value)
    {
        glUseProgram(id); // use program so that we can change uniform value
        unsigned int location = glGetUniformLocation(id, name);
        glUniform1i(location, value);
        glUseProgram(0);
    }

    void SetUniformFloat(const char* name, float value)
    {
        glUseProgram(id); // use program so that we can change uniform value
        unsigned int location = glGetUniformLocation(id, name);
        glUniform1f(location, value);
        glUseProgram(0);
    }

    void SetUniformVec3(const char* name, glm::vec3 value)
    {
        glUseProgram(id); // use program so that we can change uniform value
        unsigned int location = glGetUniformLocation(id, name);
        glUniform3fv(location, 1, &value[0]);
        glUseProgram(0);
    }

    // the program ID
    unsigned int id;
    vector<unsigned int> TextureList;

private:
    void CreateProgram(unsigned int vertex, unsigned int fragment)
    {
        int success;
        char infoLog[512];

        id = glCreateProgram();
        glAttachShader(id, vertex);
        glAttachShader(id, fragment);
        glLinkProgram(id);
        // print linking errors if any
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(id, 512, NULL, infoLog);
            cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
        }
    }

    void clean(unsigned int vertex, unsigned int fragment)
    {
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }

    class Shader
    {
    public:
        Shader(const char* file, unsigned int type)
        {
            LoadShaderSource(file);
            ShaderCompile(type);
            FreeShaderSource();
        }

        // Load shader file to program
        void LoadShaderSource(const char* file)
        {
            FILE* fp = fopen(file, "rb");
            fseek(fp, 0, SEEK_END);
            long sz = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            char* src = new char[sz + 1];
            fread(src, sizeof(char), sz, fp);
            src[sz] = '\0';
            code = new char* [1];
            code[0] = src;
        }

        void ShaderCompile(unsigned int type)
        {
            int success;
            char infoLog[512];

            // vertex Shader
            id = glCreateShader(type);
            glShaderSource(id, 1, code, NULL);
            glCompileShader(id);
            // print compile errors if any
            glGetShaderiv(id, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(id, 512, NULL, infoLog);
                cout << "ERROR::SHADER:: " << type << " ::COMPILATION_FAILED\n" << infoLog << endl;
            };
        }

        // Free shader file
        void FreeShaderSource()
        {
            delete code[0];
            delete code;
        }

        unsigned int id;

    private:
        char** code;
    };

    Shader *vertex_shader, *fragment_shader;
};
class ModelLoader
{
public:
    struct Texture;
    class Mesh;

    // model data 
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    // constructor, expects a filepath to a 3D model.
    ModelLoader(string const& path, bool gamma = false) : gammaCorrection(gamma)
    {
        loadModel(path);
    }

    // draws the model, and thus all its meshes
    void Draw(unsigned int pid)
    {
        for (unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(pid);
    }

private:
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const& path)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode* node, const aiScene* scene)
    {
        // process each mesh located at the current node
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene. 
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. h maps
        vector<Texture> hMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_h");
        textures.insert(textures.end(), hMaps.begin(), hMaps.end());

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, textures);
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for (unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if (strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if (!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }

    unsigned int TextureFromFile(const char* path, const string& directory, bool gamma = false)
    {
        string filename = string(path);
        filename = directory + '/' + filename;

        cout << filename << endl;

        unsigned int textureID;
        glGenTextures(1, &textureID);

        int w, h, nrComponents;
        unsigned char* data = stbi_load(filename.c_str(), &w, &h, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;
            else
                cout << "format not initialized" << endl;

            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            cout << "Texture failed to load at path: " << path << endl;
            stbi_image_free(data);
        }

        return textureID;
    }

    struct Vertex {
        // position
        glm::vec3 Position;
        // normal
        glm::vec3 Normal;
        // texCoords
        glm::vec2 TexCoords;
        // tangent
        glm::vec3 Tangent;
        // bitangent
        glm::vec3 Bitangent;
    };

    struct Texture {
        unsigned int id;
        string type;
        string path;
    };

    class Mesh {
    public:
        // mesh Data
        vector<Vertex>       vertices;
        vector<unsigned int> indices;
        vector<Texture>      textures;
        unsigned int VAO;

        // constructor
        Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures)
        {
            this->vertices = vertices;
            this->indices = indices;
            this->textures = textures;

            // now that we have all the required data, set the vertex buffers and its attribute pointers.
            setupMesh();
        }

        // render the mesh
        void Draw(unsigned int pid)
        {
            // bind appropriate textures
            unsigned int diffuseNr = 1;
            unsigned int specularNr = 1;
            unsigned int normalNr = 1;
            unsigned int hNr = 1;
            for (unsigned int i = 0; i < textures.size(); i++)
            {
                // retrieve texture number (the N in diffuse_textureN)
                string number;
                string name = textures[i].type;
                if (name == "texture_diffuse")
                    number = to_string(diffuseNr++);
                else if (name == "texture_specular")
                    number = to_string(specularNr++); // transfer unsigned int to stream
                else if (name == "texture_normal")
                    number = to_string(normalNr++); // transfer unsigned int to stream
                else if (name == "texture_h")
                    number = to_string(hNr++); // transfer unsigned int to stream

                // now set the sampler to the correct texture unit
                glUseProgram(pid);
                glUniform1i(glGetUniformLocation(pid, (name + number).c_str()), i);
                glUseProgram(0);

                // and finally bind the texture
                glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
                glBindTexture(GL_TEXTURE_2D, textures[i].id);
            }

            // draw mesh
            glUseProgram(pid);
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
            glUseProgram(0);

            // always good practice to set everything back to defaults once configured.
            glActiveTexture(GL_TEXTURE0);
        }

    private:
        // render data 
        unsigned int VBO, EBO;

        // initializes all the buffer objects/arrays
        void setupMesh()
        {
            // create buffers/arrays
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);
            // load data into vertex buffers
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            // A great thing about structs is that their memory layout is sequential for all its items.
            // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
            // again translates to 3/2 floats which translates to a byte array.
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

            // set the vertex attribute pointers
            // vertex Positions
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
            // vertex normals
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
            // vertex texture coords
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
            // vertex tangent
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
            // vertex bitangent
            glEnableVertexAttribArray(4);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

            glBindVertexArray(0);
        }
    };
};
class FrameBufferObject
{
public:
    unsigned int fbo, scene, vnum = 0, vao, rbo;
    ShaderProgram* program;
    int w, h;

    FrameBufferObject(int width, int height, string vs, string fs) 
        : w{ width }, h{ height }, program{ new ShaderProgram(vs.c_str(), fs.c_str()) }
    {
        VAO();
        FBO();
    }

    void VAO()
    {
        const float quad[] = {
            // positions   // texCoords
            1.0f,  -1.0f,  1.0f, 0.0f,
            -1.0f, -1.0f,  0.0f, 0.0f,
             1.0f, 1.0f,  1.0f, 1.0f,

            1.0f,  1.0f,  1.0f, 1.0f,
             -1.0f, 1.0f,  0.0f, 1.0f,
             -1.0f, -1.0f, 0.0f, 0.0f
        };

        vnum = sizeof(quad) / sizeof(float) / 4;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    void FBO()
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

        // use texture for color buffer since we are going to sample from them (read them)
        glGenTextures(1, &scene);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, scene);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene, 0);

        // use render buffer for depth and stencil since we are not going to sample from them (read them), this will be faster than texture
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        /*
        unsigned int DepthTexture, StencilTexture;

        glGenTextures(1, &DepthTexture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, DepthTexture);

        glTexImage2D
        (
            GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0,
            GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL
        );

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, DepthTexture, 0);

        glTexImage2D
        (
            GL_TEXTURE_2D, 0, GL_STENCIL_INDEX, w, h, 0,
            GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, NULL
        );

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D, DepthAndStencilTexture, 0);

        glTexImage2D
        (
            GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, w, h, 0,
            GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL
        );

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, DepthAndStencilTexture, 0);
        */
    }
    void ToScene()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glViewport(0, 0, w, h);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glEnable(GL_DEPTH_TEST);
    }
    void EndScene()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void draw()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glViewport(0, 0, w, h);
        glDisable(GL_DEPTH_TEST);

        program->BindTexture("scene", GL_TEXTURE0, scene);
        glBindVertexArray(vao);
        glUseProgram(program->id);
        glDrawArrays(GL_TRIANGLES, 0, vnum);
        program->BindTexture("scene", GL_TEXTURE0, 0);

        glEnable(GL_DEPTH_TEST);
    }
};
class CubeMap
{
public:
    unsigned int textureID = 0, vao, vnum;
    ShaderProgram* program;

    CubeMap(string dir, string format, vector<string> textures_faces, string vs, string fs)
        : program{ new ShaderProgram((dir + vs).c_str(), (dir + fs).c_str()) }
    {
        LoadCubeMap(dir, format, textures_faces);
        GenerateVAO();
    }
    void LoadCubeMap(string dir, string format, vector<string> textures_faces)
    {
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

        int w, h, nrChannels;
        for (unsigned int i = 0; i < textures_faces.size(); i++)
        {
            cout << dir + textures_faces[i] + format << endl;

            unsigned char* data = stbi_load((dir + textures_faces[i] + format).c_str(), &w, &h, &nrChannels, 0);

            GLenum type;
            if (nrChannels == 1)
                type = GL_RED;
            else if (nrChannels == 3)
                type = GL_RGB;
            else if (nrChannels == 4)
                type = GL_RGBA;
            else
                cout << "format not initialized" << endl;

            if (data)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, type, w, h, 0, type, GL_UNSIGNED_BYTE, data
                );
                stbi_image_free(data);
            }
            else
            {
                std::cout << "Cubemap tex failed to load at path: " << dir + textures_faces[i] + format << std::endl;
                stbi_image_free(data);
            }
        }
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        program->SetUniformInt("skybox", 0);
    }
    void GenerateVAO()
    {
        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
        vnum = sizeof(skyboxVertices) / sizeof(float) / 3;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao); // glVertexAttribPointer()... would change content of VAO bound currently, so we should bind our VAO first
        // A vertex array object stores the following:

        unsigned int vbo;
        glGenBuffers(1, &vbo); // generate a buffer
        glBindBuffer(GL_ARRAY_BUFFER, vbo); // set buffer to opengl content
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

        // position attribute
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindVertexArray(0);
    }
    void draw()
    {
        glDepthMask(GL_FALSE);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
        glUseProgram(program->id);
        glBindVertexArray(vao);

        glDrawArrays(GL_TRIANGLES, 0, vnum);

        glBindVertexArray(0);
        glUseProgram(0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

        glDepthMask(GL_TRUE);
    }
};

// the whole drawing pipeline and all needed objects should be contained in one config
class pipeline
{
public:
    ShaderProgram *program;
    ModelLoader *loader;
    FrameBufferObject *fbo;

    void draw(unsigned int vao, unsigned int num)
    {
        glUseProgram(program->id);
        glBindVertexArray(vao);

        glDrawArrays(GL_TRIANGLES, 0, num);
        
        glBindVertexArray(0);
        glUseProgram(0);
    }
};

