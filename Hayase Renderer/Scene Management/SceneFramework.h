#ifndef SCENEFRAMEWORK_H
#define SCENEFRAMEWORK_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>


#define _GET_GL_ERROR   { GLenum err = glGetError(); std::cout << "[OpenGL Error] " << glewGetErrorString(err) << std::endl; }

namespace Hayase
{
    class Scene
    {

    public:
        Scene();
        Scene(int windowWidth, int windowHeight);
        virtual ~Scene();

        // Public methods

        // Init: called once when the scene is initialized
        virtual int Init();

        // LoadAllShaders: This is the placeholder for loading the shader files
        virtual void LoadAllShaders();

        // Display : encapsulates per-frame behavior of the scene
        virtual int Display();

        // preRender : called to setup stuff prior to rendering the frame
        virtual int preRender();

        // Render : per frame rendering of the scene
        virtual int Render();

        // postRender : Any updates to calculate after current frame
        virtual int postRender();

        // ProcessInput : Update according to user input
        virtual void ProcessInput(GLFWwindow* w, float dt);

        // ProcessMouse : Update mouse (for the camera)
        virtual void ProcessMouse(float x, float y);

        // cleanup before destruction
        virtual void CleanUp();

    protected:
        int _windowHeight, _windowWidth;
    };
}

#endif
