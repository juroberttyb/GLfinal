#include "include/includer.h"

int default_w = 1440, default_h = 900;

struct Light
{
    vec3 pos = vec3(-20.0, 20.0, -20.0);
    vec3 center = vec3(0.0, 0.0, 0.0);
    vec3 up = vec3(0.0, 1.0, 0.0);
} light;
struct ShadowProj
{
    float near = 0.0, far = 500.0, range = 150.0;
} shadow;

class _window : public Window
{
public:
    bool Ipress = false, Fpress = false, Tpress = false;
    int mode = 0, post_effect = 0, on = 0;

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

        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !Tpress)
        {
            on = (on + 1) % 2;
            Tpress = true;
        }
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE)
        {
            Tpress = false;
        }

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        {
            light.pos.x += 1;
        }
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
        {
            light.pos.x -= 1;
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
    };
    class ShadowProg : public pipeline
    {
    public:
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
            }
            void draw(GLint vao, GLint vnum, mat4 model)
            {
                program->SetUniformMat("mvp", vp * model);
                pipeline::draw(vao, vnum);
            }
        } depthmap;

        ShadowProg()
        {
            program = new ShaderProgram("include/shadow/shader.vs", "include/shadow/shader.fs");
        }
        void draw(_window* window, unsigned int vao, int vnum, mat4 model, int shapeonly = 0)
        {
            mat4 shadow_matrix = depthmap.sbpv * model;
            program->SetUniformMat("shadow_matrix", shadow_matrix);
            program->BindTexture("shadow_tex", GL_TEXTURE0, depthmap.map);
            program->SetUniformMat("mvp", window->project * window->view * model);
            program->SetUniformInt("shapeonly", shapeonly);

            pipeline::draw(vao, vnum);
        }
    };
    // assimp obj for normal mapping
    class Assimp_obj : public pipeline
    {
    public:
        mat4 model = mat4(1.0f);

        Assimp_obj(const char *vs, const char *fs, const char *path)
        {
            program = new ShaderProgram(vs, fs);
            float scaling = 0.1f;
            model = glm::scale(glm::mat4(1.0f), glm::vec3(scaling, scaling, scaling));

            loader = new ModelLoader(path);
        }
        void draw(_window* window)
        {
            program->SetUniformVec3("light_pos", light.pos);
            program->SetUniformInt("on", window->on);

            program->SetUniformMat("model", model);
            program->SetUniformMat("view", window->view);
            program->SetUniformMat("project", window->project);

            loader->Draw(program->id);
        }
    };
    // tiny obj for shadow mapping
    class Tiny_obj : public pipeline
    {
    public:
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
        } quad;

        TinyOjectLoader loader;
        glm::mat4 model = glm::mat4(1.0f);

        Tiny_obj(const char* vs, const char* fs, const char* path)
        {
            program = new ShaderProgram(vs, fs);

            loader.load(path);
        }
        void draw(_window* window, unsigned int cubemapid)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapid);

            program->SetUniformVec3("light_pos", light.pos);
            program->SetUniformMat("view", window->view);
            program->SetUniformMat("project", window->project);
            program->SetUniformMat("model", model);
            pipeline::draw(loader.vao, loader.vnum);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
        }
    };

    Frame* combine, * shad, * noshad;
    ShadowProg sp;
    Sky cubemap;
    // tiny obj for obj used with shadow mapping
    Tiny_obj city = Tiny_obj("obj/metro_city/shader.vs", "obj/metro_city/shader.fs", "obj/metro_city/Metro city.obj");
    // assimp obj for obj used with normal mapping
    Assimp_obj oak = Assimp_obj("obj/oak/shader.vs", "obj/oak/shader.fs", "obj/oak/white_oak.obj");

    DiffRender(string vs = string("obj/DiffRender/shader.vs"), string fs = string("obj/DiffRender/shader.fs"))
        : Frame(vs, fs), 
        combine{ new Frame() }, 
        shad{ new Frame() }, 
        noshad{ new Frame() }
    {
        mat4 model = mat4(1.0f);
        model = translate(model, glm::vec3(0, 2.5, 0));
        model = scale(model, glm::vec3(0.1, 0.1, 0.1));

        city.model = model;

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0, -0.6, 0));
        model = glm::scale(model, glm::vec3(200, 1, 200));

        city.quad.model = model;
    }

    void Flow(_window* window)
    {
        DefaultFlow(window);
        ShadowFlow(window);
        NoShadowFlow(window);
    }
    void DrawSceneTexture(_window* window)
    {
        Flow(window);

        FrameBufferObject::ToScene();

        program->BindTexture("combine", GL_TEXTURE1, combine->scene);
        program->BindTexture("shad", GL_TEXTURE2, shad->scene);
        program->BindTexture("noshad", GL_TEXTURE3, noshad->scene);
        program->SetUniformInt("mode", window->mode);

        glViewport(0, 0, window->w, window->h);
        glUseProgram(program->id);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, vnum);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

private:
    void DefaultFlow(_window* window)
    {
        combine->ToScene();

        cubemap.draw(window);
        city.draw(window, cubemap.textureID);
        oak.draw(window);

        combine->EndScene();
    }
    void ShadowFlow(_window* window)
    {
        sp.depthmap.ToScene();

        sp.depthmap.draw(city.loader.vao, city.loader.vnum, city.model);
        sp.depthmap.draw(city.quad.vao, city.quad.vnum, city.quad.model);

        sp.depthmap.EndScene();



        shad->ToScene();

        sp.draw(window, city.loader.vao, city.loader.vnum, city.model);
        sp.draw(window, city.quad.vao, city.quad.vnum, city.quad.model);

        shad->EndScene();
    }
    void NoShadowFlow(_window* window)
    {
        noshad->ToScene();

        sp.draw(window, city.loader.vao, city.loader.vnum, city.model, 1);
        sp.draw(window, city.quad.vao, city.quad.vnum, city.quad.model, 1);

        noshad->EndScene();
    }
};

void loop()
{
    _window window(default_w, default_h);
    DiffRender diff;
    Frame PostEffect(diff.scene, string("include/PostEffect/shader.vs"), string("include/PostEffect/shader.fs"));

    while (window.update())
    {
        diff.DrawSceneTexture(&window);
        PostEffect.draw(&window);
    }
}

int main()
{
    loop();
    return 0;
}
