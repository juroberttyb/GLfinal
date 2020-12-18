#pragma once

#include <gtc/matrix_transform.hpp>

// viewport transform
// The output of the vertex shader requires the coordinates to be in clip-space which is what we just did with the mvp transformation matrices. 
// OpenGL then performs perspective division on the clip-space coordinates to transform them to normalized-device coordinates. 
// OpenGL then uses the parameters from glViewPort to map the normalized-device coordinates to screen coordinates where each coordinate corresponds to a point on your screen 
// (in our case a 800x600 screen). This process is called the viewport transform.

// rotation order of 6 kind to be written xyz, xzy..., are thay 

glm::mat4 Project(float fov)
{
    glm::mat4 projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);

    // glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    // Its first parameter defines the fov value, that stands for field of view and sets how large the viewspace is. 
    // For a realistic view it is usually set to 45 degrees, but for more doom-style results you could set it to a higher value. 
    // The second parameter sets the aspect ratio which is calculated by dividing the viewport's width by its height. 
    // The third and fourth parameter set the near and far plane of the frustum. We usually set the near distance to 0.1 and the far distance to 100.0. 
    // All the vertices between the near and far plane and inside the frustum will be rendered.

    // Whenever the near value of your perspective matrix is set too high (like 10.0), OpenGL will clip all coordinates close to the camera (between 0.0 and 10.0),
    // which can give a visual result you maybe have seen before in videogames where you could see through certain objects when moving uncomfortably close to them.



    // glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, 0.1f, 100.0f); // orthogonal transformation
    
    // The first two parameters specify the left and right coordinate of the frustum
    // and the third and fourth parameter specify the bottom and top part of the frustum. 
    // With those 4 points we've defined the size of the near and far planes
    // and the 5th and 6th parameter then define the distances between the near and far plane. 
    // This specific projection matrix transforms all coordinates between these x, y and z range values to normalized device coordinates.
    // An orthographic projection matrix directly maps coordinates to the 2D plane that is your screen

    return projection;
}

glm::mat4 View(glm::vec3 cameraPos, glm::vec3 cameraFront, glm::vec3 cameraUp)
{
    // note that we're translating the scene in the reverse direction of where we want to move
    // glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    return view;
}

glm::mat4 Model(glm::mat4 model, glm::vec3 translate, glm::vec3 rotate, glm::vec3 scale)
{
    model = glm::scale(model, scale);

    model = glm::rotate(model, rotate.x, glm::vec3(1, 0, 0));
    model = glm::rotate(model, rotate.y, glm::vec3(0, 1, 0));
    model = glm::rotate(model, rotate.z, glm::vec3(0, 0, 1));

    model = glm::translate(model, translate);

    return model;
}

/*
glm::mat4 MVP(glm::mat4 mvp, glm::vec3 translate, glm::vec3 rotate, glm::vec3 scale)
{
    mvp = Model(mvp, translate, rotate, scale);
    mvp = View();
    mvp = Project();

    return mvp;
}
*/

