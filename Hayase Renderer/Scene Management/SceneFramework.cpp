#include "SceneFramework.h"

namespace Hayase
{
    SceneFramework::SceneFramework() : _windowWidth(100), _windowHeight(100)
    {

    }

    SceneFramework::SceneFramework(int windowWidth, int windowHeight)
    {
        _windowHeight = windowHeight;
        _windowWidth = windowWidth;
    }

    SceneFramework::~SceneFramework()
    {
        CleanUp();
    }

    // Public methods

    // Init: called once when the scene is initialized
    int SceneFramework::Init()
    {
        return -1;
    }

    // LoadAllShaders: This is the placeholder for loading the shader files
    void SceneFramework::LoadAllShaders()
    {
        return;
    }


    // preRender : called to setup stuff prior to rendering the frame
    int SceneFramework::preRender()
    {
        return -1;
    }

    // Render : per frame rendering of the scene
    int SceneFramework::Render()
    {
        return -1;
    }

    // postRender : Any updates to calculate after current frame
    int SceneFramework::postRender()
    {
        return -1;
    }

    // ProcessInput : Update according to user input
    void SceneFramework::ProcessInput(GLFWwindow* w, float dt)
    {
        return;
    }

    // ProcessMouse : Update mouse (for the camera)
    void SceneFramework::ProcessMouse(float x, float y)
    {
        return;
    }

    // CleanUp : clean up resources before destruction
    void SceneFramework::CleanUp()
    {
        return;
    }

    // Display : Per-frame execution of the scene
    int SceneFramework::Display()
    {
        preRender();
        Render();
        postRender();

        return -1;
    }
}