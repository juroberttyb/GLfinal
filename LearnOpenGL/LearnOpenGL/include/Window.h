#pragma once
#ifndef WINDOW_H
#define WINDOW_H

#include <glfw3.h> // for creating window
#include <iostream>
#include <glm.hpp>
#include <cmath>
#include "transform.h"
#include "pipeline.h"

using namespace glm;

// this is a class designed only to be called once, only one object should be created and maintained
class Window
{
public:
    static struct Camera {
        glm::vec3 pos = glm::vec3(0.0f, 0.0f, 5.0f);
        glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        float fov = 100.0f, camera_speed = 8.0f;
    } camera;
    static struct TrackBall
    {
        bool firstMouse;
        float lastX, lastY;
        float pitch, yaw;
    } trackball;
    static struct Cursor
    {
        double x = 0, y = 0;
    } cursor;

    glm::mat4 view, project;
    GLFWwindow* window;
    int w, h;
    float exposure = 1.0f;

    Window(int width, int height)
    {
        glfwInit();

        w = width;
        h = height;
        trackball.firstMouse = true;
        trackball.lastX = w / 2.0f;
        trackball.lastY = h / 2.0f;
        trackball.pitch = 0.0f;
        trackball.yaw = -90.0f;

        view = View(camera.pos, camera.front, camera.up);
        project = Project(camera.fov);

        create_window();
        glfwMakeContextCurrent(window); // tell GLFW to make the context of our window the main context on the current thread
        register_callbacks();
        // SetAttribute();

        // OpenGL function could only be used after glad initialized successfully
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) // pass GLAD the function to load the address of the OpenGL function pointers which is OS-specific, glfwGetProcAddress defines the correct function based on which OS we're compiling for
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            exit(1);
        }

        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, w, h); // tell OpenGL the size of the rendering window so OpenGL knows how we want to display the data and coordinates with respect to the window
    }
    void create_window()
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        window = glfwCreateWindow(w, h, "LearnOpenGL", NULL, NULL);
        if (window == NULL)
        {
            std::cout << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            exit(0);
        }
    }
    void register_callbacks()
    {
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // register a callback function

        glfwSetCursorPosCallback(window, mouse_callback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        glfwSetScrollCallback(window, scroll_callback);
    }
    void processInput()
    {
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) // if it's not pressed, glfwGetKey returns GLFW_RELEASE
            glfwSetWindowShouldClose(window, true);

        const float cameraSpeed = camera.camera_speed * deltaTime; // adjust accordingly

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.pos += cameraSpeed * camera.front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.pos -= cameraSpeed * camera.front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.pos -= glm::normalize(glm::cross(camera.front, camera.up)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.pos += glm::normalize(glm::cross(camera.front, camera.up)) * cameraSpeed;

        view = View(camera.pos, camera.front, camera.up);
        project = Project(camera.fov);
    }
    void clear(bool state)
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        if (state)
        {
            glfwTerminate(); // clean/delete all of GLFW's resources that were allocated
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

private:
    static void framebuffer_size_callback(GLFWwindow* window, int w, int h)
    {
        glViewport(0, 0, w, h);
    }
    static void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        cursor.x = xpos;
        cursor.y = ypos;

        if (trackball.firstMouse)
        {
            trackball.lastX = xpos;
            trackball.lastY = ypos;
            trackball.firstMouse = false;
        }

        float xoffset = xpos - trackball.lastX;
        float yoffset = trackball.lastY - ypos;
        trackball.lastX = xpos;
        trackball.lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        trackball.yaw += xoffset;
        trackball.pitch += yoffset;

        if (trackball.pitch > 89.0f)
            trackball.pitch = 89.0f;
        if (trackball.pitch < -89.0f)
            trackball.pitch = -89.0f;

        glm::vec3 direction;
        direction.x = cos(glm::radians(trackball.yaw)) * cos(glm::radians(trackball.pitch));
        direction.y = sin(glm::radians(trackball.pitch));
        direction.z = sin(glm::radians(trackball.yaw)) * cos(glm::radians(trackball.pitch));
        camera.front = glm::normalize(direction);
    }
    static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
    {
        float constant = 0.5f;

        if (yoffset > 0)
        {
            camera.camera_speed += constant;
        }
        if (yoffset < 0 && camera.camera_speed - constant > 0)
        {
            camera.camera_speed -= constant;
        }
    }

    float deltaTime = 0.0f;
    float currentFrame = 0.0f;
    float lastFrame = 0.0f;
};

Window::Camera Window::camera;
Window::TrackBall Window::trackball;
Window::Cursor Window::cursor;

#endif