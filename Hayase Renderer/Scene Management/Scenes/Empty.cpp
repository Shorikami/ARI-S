#include <hyspch.h>

#include "Empty.h"

namespace Hayase
{
    EmptyScene::EmptyScene(int windowWidth, int windowHeight)
        : Scene(windowWidth, windowHeight)
        , Layer("EmptyScene")
    {
    }

    EmptyScene:: ~EmptyScene()
    {

    }

    int EmptyScene::Init()
    {
        return 0;
    }

    void EmptyScene::CleanUp()
    {
    }

    int EmptyScene::PreRender(float frame)
    {
        return 0;
    }

    int EmptyScene::Render()
    {
        glClearColor(0.1f, 1.0f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return 0;
    }

    int EmptyScene::PostRender()
    {
        return 0;
    }

    void EmptyScene::OnAttach()
    {

    }

    void EmptyScene::OnDetach()
    {

    }

    void EmptyScene::Update(DeltaTime dt)
    {
        std::cout << GetName() << " updated" << std::endl;
    }

    void EmptyScene::OnEvent(Event& e) 
    {
        std::cout << e.ToString() << std::endl;
    }
}