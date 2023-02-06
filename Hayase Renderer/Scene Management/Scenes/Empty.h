#ifndef EMPTYSCENE_H
#define EMPTYSCENE_H

#include "SceneFramework.h"
#include "Layer.h"

namespace Hayase
{
    class Entity;

	class EmptyScene : public Scene, public Layer
	{
    public:
        EmptyScene() = default;
        EmptyScene(int windowWidth, int windowHeight);
        virtual ~EmptyScene();


    public:
        int Init() override;
        void CleanUp() override;

        int PreRender() override;
        int Render() override;
        int PostRender() override;

    public:
        void OnAttach() override;
        void OnDetach() override;
        void Update(DeltaTime dt) override;
        void OnEvent(Event& e) override;
	};
}

#endif