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
    float near = 0.0, far = 500.0, range = 250.0;
} shadow;

class _window : public Window
{
public:
    bool Ipress = false, Fpress = false;
    int mode = 0, post_effect = 0;

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
            post_effect = (post_effect + 1) % 4;
            Fpress = true;
        }
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE)
        {
            Fpress = false;
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
    Frame(string vs = string("include/fbo/shader.vs"), string fs = string("include/fbo/shader.fs"), bool hdr = false)
        : FrameBufferObject(default_w, default_h, vs, fs)
    {
        if (hdr)
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);

            glGenTextures(1, &scene);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, scene);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, scene, 0);
        }
    }
    void draw(_window* window)
    {
        program->SetUniformInt("xpos", window->cursor.x);
        program->SetUniformInt("ypos", window->h - window->cursor.y);
        program->SetUniformFloat("time", glfwGetTime());
        program->SetUniformFloat("exposure", window->exposure);
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

                mat4 proj = ortho(-shadow.range, shadow.range, -shadow.range, shadow.range, shadow.near, shadow.far);
                mat4 view = lookAt(light.pos, light.center, light.up);
                vp = proj * view;

                mat4 sb = translate(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f)) * scale(mat4(1.0f), vec3(0.5f, 0.5f, 0.5f));
                sbpv = sb * vp;
            } 
            void ToScene()
            {
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
    class OBJ : public pipeline
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

        OBJ(string vs, string fs, string load_path)
        {
            program = new ShaderProgram(vs.c_str(), fs.c_str());

            loader.load(load_path.c_str());
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
    OBJ city = OBJ(string("obj/metro_city/shader.vs"), string("obj/metro_city/shader.fs"), string("obj/metro_city/Metro city.obj"));

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

class Unitest : public pipeline
{
public:
    unsigned int vao, vnum;
    mat4 model = mat4(1.0f);;

    Unitest()
    {
        program = new ShaderProgram("obj/NormalMap/blinphong/shader.vs", "obj/NormalMap/blinphong/shader.fs");
        program->TextureFromFile(string("obj/NormalMap/brickwall.jpg"));
        program->TextureFromFile(string("obj/NormalMap/brickwall_normal.jpg"));
        program->TextureFromFile(string("obj/NormalMap/container2_specular.png"));

        CubeVAO();

        // model = scale(model, vec3(1.0, 1.0, 1.0));
        program->SetUniformMat("model", model);

        program->SetUniformVec3("material.ambient", vec3(1.0f, 0.5f, 0.31f));
        program->SetUniformVec3("material.diffuse", vec3(1.0f, 0.5f, 0.31f));
        program->SetUniformVec3("material.specular", vec3(0.5f, 0.5f, 0.5f));
        program->SetUniformFloat("material.shininess", 32.0f);

        dirlight_set();
        pointlight_set();
        spotlight_set();
    }
    void CubeVAO()
    {
        float vertices[] = {
            // positions          // normals           // texture coords
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
             0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
            -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
             0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
             0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
             0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
             0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
             0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
            -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
            -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
        };

        int float_a_row = 8;
        vnum = sizeof(vertices) / sizeof(float) / float_a_row;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao); // glVertexAttribPointer()... would change content of VAO bound currently, so we should bind our VAO first
        // A vertex array object stores the following:

        unsigned int VBO;
        glGenBuffers(1, &VBO); // generate a buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO); // set buffer to opengl content

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, float_a_row * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, float_a_row * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // tex coord
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, float_a_row * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
    }
    void dirlight_set()
    {
        // DirLight setting
        struct DirLight {
            vec3 direction;

            vec3 ambient;
            vec3 diffuse;
            vec3 specular;
        };

        program->SetUniformVec3("dirLight.direction", vec3(-0.2f, -1.0f, -0.3f));

        program->SetUniformVec3("dirLight.ambient", vec3(0.2f, 0.2f, 0.2f));
        program->SetUniformVec3("dirLight.diffuse", vec3(0.5f, 0.5f, 0.5f)); // darken diffuse light a bit
        program->SetUniformVec3("dirLight.specular", vec3(1.0f, 1.0f, 1.0f));
    }
    void pointlight_set()
    {
        // PointLight setting
        struct PointLight {
            vec3 position;

            float constant;
            float linear;
            float quadratic;

            vec3 ambient;
            vec3 diffuse;
            vec3 specular;
        };

        string pointlight = "pointLights[",
            position = "].position",
            ambient = "].ambient",
            diffuse = "].diffuse",
            specular = "].specular",
            constant = "].constant",
            linear = "].linear",
            quadratic = "].quadratic";

        int lightcount = 4;
        vec3 pointLightPositions[] = {
            vec3(0.7f,  0.2f,  2.0f),
            vec3(2.3f, -3.3f, -4.0f),
            vec3(-4.0f,  2.0f, -12.0f),
            vec3(0.0f,  0.0f, -3.0f)
        };

        for (int i = 0; i < lightcount; i++)
        {
            program->SetUniformVec3((pointlight + to_string(i) + position).c_str(), pointLightPositions[i]);

            program->SetUniformVec3((pointlight + to_string(i) + ambient).c_str(), vec3(0.2f, 0.2f, 0.2f));
            program->SetUniformVec3((pointlight + to_string(i) + diffuse).c_str(), vec3(0.5f, 0.5f, 0.5f));
            program->SetUniformVec3((pointlight + to_string(i) + specular).c_str(), vec3(1.0f, 1.0f, 1.0f));

            program->SetUniformFloat((pointlight + to_string(i) + constant).c_str(), 1.0f);
            program->SetUniformFloat((pointlight + to_string(i) + linear).c_str(), 0.09f);
            program->SetUniformFloat((pointlight + to_string(i) + quadratic).c_str(), 0.032f);
        }
    }
    void spotlight_set()
    {
        // SpotLight setting
        struct SpotLight {
            vec3  position;
            vec3  direction;
            float cutOff;
            float outerCutOff;

            vec3 ambient;
            vec3 diffuse;
            vec3 specular;
        };

        program->SetUniformVec3("spotLight.direction", vec3(0.0f, 0.0f, -1.0f));
        program->SetUniformVec3("spotLight.position", vec3(0.0f, 0.0f, 0.0f));

        program->SetUniformVec3("spotLight.ambient", vec3(0.2f, 0.2f, 0.2f));
        program->SetUniformVec3("spotLight.diffuse", vec3(0.5f, 0.5f, 0.5f)); // darken diffuse light a bit
        program->SetUniformVec3("spotLight.specular", vec3(1.0f, 1.0f, 1.0f));

        program->SetUniformFloat("spotLight.constant", 1.0f);
        program->SetUniformFloat("spotLight.linear", 0.09f);
        program->SetUniformFloat("spotLight.quadratic", 0.032f);

        program->SetUniformFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        program->SetUniformFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
    }
    void spotlight_update(Window* window)
    {
        program->SetUniformVec3("spotLight.position", window->camera.pos);
        program->SetUniformVec3("spotLight.direction", window->camera.front);
    }
    void draw_one(Window* window)
    {
        spotlight_update(window);

        program->SetUniformVec3("ViewPos", window->camera.pos);

        program->SetUniformMat("view", window->view);
        program->SetUniformMat("project", window->project);

        program->BindTexture("material.diffuse", GL_TEXTURE0, program->TextureList[0]);
        program->BindTexture("normalMap", GL_TEXTURE1, program->TextureList[1]);
        program->BindTexture("material.specular", GL_TEXTURE2, program->TextureList[2]);

        pipeline::draw(vao, vnum);

        program->BindTexture("material.specular", GL_TEXTURE2, 0);
        program->BindTexture("normalMap", GL_TEXTURE1, 0);
        program->BindTexture("material.diffuse", GL_TEXTURE0, 0);
    }

    void draw(Window* window)
    {
        vec3 cubePositions[] = {
            glm::vec3(0.0f,  0.0f,  0.0f),
            glm::vec3(2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f,  3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f,  2.0f, -2.5f),
            glm::vec3(1.5f,  0.2f, -1.5f),
            glm::vec3(-1.3f,  1.0f, -1.5f)
        };

        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            program->SetUniformMat("model", model);

            draw_one(window);
        }
    }
};

void loop()
{
    _window window(default_w, default_h);
    DiffRender diff;
    Frame PostEffect(string("include/PostEffect/shader.vs"), string("include/PostEffect/shader.fs"), true);

    // Unitest cube;

    while (window.update())
    {
        // cube.draw(&window);
        // /*
        diff.DrawSceneTexture(&window);

        PostEffect.scene = diff.scene;
        PostEffect.draw(&window);
        // */
    }
}

int main()
{
    loop();
    return 0;
}
