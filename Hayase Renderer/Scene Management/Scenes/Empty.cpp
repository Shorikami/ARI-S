#include <hyspch.h>

#include "Empty.h"
#include "Entity.h"

namespace Hayase
{
    EmptyScene::EmptyScene(int windowWidth, int windowHeight)
        : Scene(windowWidth, windowHeight)
        , Layer("EmptyScene")
    {
        Init();
    }

    EmptyScene:: ~EmptyScene()
    {

    }

    int EmptyScene::Init()
    {
        Entity e = CreateEntity("test");
        e.AddComponent<MeshComponent>(std::string("Materials/Models/BA/Shiroko/Mesh/Shiroko_Original_Weapon.obj"));

        return Scene::Init();
    }

    void EmptyScene::CleanUp()
    {
    }

    int EmptyScene::PreRender()
    {
        return 0;
    }

    int EmptyScene::Render()
    {
        glClearColor(0.1f, 1.0f, 0.5f, 1.0f);
        m_SceneFBO->Activate();
        m_SceneFBO->Unbind();
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
        m_DT = dt.GetSeconds();

        // ... then render
        Display();
    }

    void EmptyScene::OnEvent(Event& e) 
    {
        std::cout << e.ToString() << std::endl;
    }
}