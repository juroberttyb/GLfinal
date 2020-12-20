#include "include/includer.h"

int default_w = 1440, default_h = 900;

struct Light
{
    vec3 pos = vec3(0.0, 10.0, -10.0);
    vec3 center = vec3(0.0, 0.0, 0.0);
    vec3 up = vec3(0.0, 1.0, 0.0);
} light;
struct ShadowProj
{
    float near = 0.0, far = 100, range = 25.0;
    mat4 vp;
    unsigned int scene;
} shadow;

class _window : public Window
{
public:
    bool Ipress = false, Fpress = false, Npress = false;
    int mode = 0, post_effect = 0, NormalOn = 1;

    _window(int w, int h)
        : Window(w, h)
    {

    }

    void processInput()
    {
        Window::processInput();

        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS && !Ipress)
        {
            mode = (mode + 1) % 4;
            Ipress = true;
        }
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE)
        {
            Ipress = false;
        }

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !Fpress)
        {
            post_effect = (post_effect + 1) % 3;
            Fpress = true;
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
        {
            Fpress = false;
        }

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !Npress)
        {
            NormalOn = (NormalOn + 1) % 2;
            Npress = true;
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
        {
            Npress = false;
        }

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            light.pos = rotate(mat4(1.0f), 0.1f, vec3(0.0, 1.0, 0.0)) * vec4(light.pos, 1.0f);
        }
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            light.pos = rotate(mat4(1.0f), -0.1f, vec3(0.0, 1.0, 0.0)) * vec4(light.pos, 1.0f);
        }
    }
    bool update()
    {
        glfwPollEvents();
        glfwSwapBuffers(window);
        processInput();

        bool state = glfwWindowShouldClose(window);
        clear(state);

        return !state;
    }
};
class Frame : public FrameBufferObject
{
public:
    Frame(string vs = string("include/fbo/shader.vs"), string fs = string("include/fbo/shader.fs"))
        : FrameBufferObject(default_w, default_h, vs, fs)
    {
    }
    Frame(unsigned int FromScene, string vs = string("include/fbo/shader.vs"), string fs = string("include/fbo/shader.fs"))
        : FrameBufferObject(default_w, default_h, vs, fs)
    {
        scene = FromScene;
    }
    void draw(_window* window)
    {
        program->SetUniformInt("PostEffect", window->post_effect);
        FrameBufferObject::draw();
    }
};
class DiffRender : public Frame
{
public:
    class Sky : public CubeMap
    {
    public:
        Sky() : CubeMap(string("obj/sky/"), string(".jpg"),
            vector<string>{string("posx"), string("negx"), string("posy"), string("negy"), string("posz"), string("negz")},
            string("sky.vs"), string("sky.fs"))
        {
        }

        void draw(_window* window)
        {
            glm::mat4 view = glm::mat4(glm::mat3(window->view));

            program->SetUniformMat("view", view);
            program->SetUniformMat("project", window->project);

            CubeMap::draw();
        }
    }
    sky;
    class DepthMap : public pipeline
    {
    public:
        GLuint fbo;
        GLuint map;
        mat4 vp, sbpv;
        int size = 4096;

        DepthMap()
        {
            program = new ShaderProgram("include/shadow/shadow.vs", "include/shadow/shadow.fs");

            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);

            glGenTextures(1, &map);
            glBindTexture(GL_TEXTURE_2D, map);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_REPEAT);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, map, 0);

            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        void ToScene()
        {
            mat4 proj = ortho(-shadow.range, shadow.range, -shadow.range, shadow.range, shadow.near, shadow.far);
            mat4 view = lookAt(light.pos, light.center, light.up);
            vp = proj * view;

            mat4 sb = translate(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f)) * scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f));
            sbpv = sb * vp;

            glUseProgram(program->id);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glClear(GL_DEPTH_BUFFER_BIT);

            glViewport(0, 0, size, size);

            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(4.0f, 4.0f);
            glUseProgram(0);
        }
        void EndScene()
        {
            glDisable(GL_POLYGON_OFFSET_FILL);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, default_w, default_h);
        }
        void draw(GLint vao, GLint vnum, mat4 model)
        {
            program->SetUniformMat("mvp", vp * model);
            pipeline::draw(vao, vnum);
        }
    } 
    dp;
    class Assimp_obj : public pipeline
    {
    public:
        mat4 model = mat4(1.0f);

        Assimp_obj(const char *path)
        {
            program = new ShaderProgram("include/AssimpShader/shader.vs", "include/AssimpShader/shader.fs");
            loader = new ModelLoader(path);
        }
        void draw(_window* window)
        {
            program->SetUniformVec3("light_pos", light.pos);
            
            program->SetUniformInt("NormalOn", window->NormalOn);
            program->BindTexture("shadowMap", GL_TEXTURE5, shadow.scene);
            glActiveTexture(GL_TEXTURE0);

            program->SetUniformMat("model", model);
            program->SetUniformMat("view", window->view);
            program->SetUniformMat("project", window->project);
            program->SetUniformMat("lightSpaceMatrix", shadow.vp);

            loader->Draw(program->id);

            program->BindTexture("shadowMap", GL_TEXTURE5, 0);
            glActiveTexture(GL_TEXTURE0);
        }
    }
    oak = Assimp_obj("obj/oak/white_oak.obj");
    class Tiny_obj : public pipeline
    {
    public:
        TinyOjectLoader loader;
        glm::mat4 model = glm::mat4(1.0f);

        Tiny_obj(const char* path)
        {
            program = new ShaderProgram("include/TinyobjShader/shader.vs", "include/TinyobjShader/shader.fs");

            loader.load(path);
        }
        void draw(_window* window, unsigned int cubemapid)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapid);

            program->SetUniformVec3("light_pos", light.pos);

            program->BindTexture("shadowMap", GL_TEXTURE5, shadow.scene);
            glActiveTexture(GL_TEXTURE0);

            program->SetUniformMat("view", window->view);
            program->SetUniformMat("project", window->project);
            program->SetUniformMat("model", model);
            program->SetUniformMat("lightSpaceMatrix", shadow.vp);

            pipeline::draw(loader.vao, loader.vnum);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            program->BindTexture("shadowMap", GL_TEXTURE5, 0);
            glActiveTexture(GL_TEXTURE0);
        }
    } 
    city = Tiny_obj("obj/metro_city/Metro city.obj");
    class Quad : public pipeline
    {
    public:
        unsigned int vao, vnum;
        glm::mat4 model = mat4(1.0f);

        Quad()
        {
            program = new ShaderProgram("obj/quad/shader.vs", "obj/quad/shader.fs");

            CubeLoad();
        }
        void CubeLoad()
        {
            float vertices[] = {
                    1.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f,
                    1.0f,  0.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
                    -1.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f,
                    -1.0f,  0.0f,  1.0f, 0.0f, 1.0f, 0.0f,
                    -1.0f,  0.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
                    1.0f,  0.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
            };

            vnum = sizeof(vertices) / sizeof(float) / 6;

            glGenVertexArrays(1, &vao);
            glBindVertexArray(vao);

            unsigned int VBO;
            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glBindVertexArray(0);
        }
        void draw(_window* window)
        {
            program->SetUniformMat("model", model);
            program->SetUniformMat("view", window->view);
            program->SetUniformMat("project", window->project);

            pipeline::draw(vao, vnum);
        }
    } 
    quad;

    DiffRender(string vs = string("obj/DiffRender/shader.vs"), string fs = string("obj/DiffRender/shader.fs"))
        : Frame(vs, fs)
    {
        float scaling = 0.01f;
        mat4 model = scale(mat4(1.0f), vec3(scaling, scaling, scaling));
        city.model = model;

        model = scale(mat4(1.0f), vec3(scaling, scaling, scaling));
        oak.model = model;
    }

    void DrawToScene(_window* window)
    {
        dp.ToScene();

        dp.draw(city.loader.vao, city.loader.vnum, city.model);
        for (int i = 0; i < oak.loader->meshes.size(); i++)
            dp.draw(oak.loader->meshes[i].VAO, oak.loader->meshes[i].vertices.size(), oak.model);

        dp.EndScene();

        shadow.vp = dp.vp;
        shadow.scene = dp.map;

        ToScene();

        sky.draw(window);
        city.draw(window, sky.textureID);
        oak.draw(window);

        EndScene();
    }
};

class Gbuffer
{
public:
    unsigned int gAlbedoSpec;

    void create()
    {
        unsigned int gBuffer;
        glGenFramebuffers(1, &gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        unsigned int gPosition, gNormal, gColorSpec;

        // - position color buffer
        glGenTextures(1, &gPosition);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, default_w, default_h, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

        // - normal color buffer
        glGenTextures(1, &gNormal);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, default_w, default_h, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

        // - color + specular color buffer
        glGenTextures(1, &gAlbedoSpec);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, default_w, default_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

        // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);

        unsigned int rbo;
        // use render buffer for depth and stencil since we are not going to sample from them (read them), this will be faster than texture
        glGenRenderbuffers(1, &rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, default_w, default_h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo);

        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
};

void loop()
{
    _window window(default_w, default_h);
    DiffRender diff;
    Frame PostEffect(diff.scene, string("include/PostEffect/shader.vs"), string("include/PostEffect/shader.fs"));

    while (window.update())
    {
        diff.DrawToScene(&window);
        PostEffect.draw(&window);
    }
}

int main()
{
    loop();
    return 0;
}
