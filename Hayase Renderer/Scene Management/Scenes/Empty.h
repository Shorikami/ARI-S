#ifndef EMPTYSCENE_H
#define EMPTYSCENE_H

#include "../SceneFramework.h"
#include "../../Utilities/Layers/Layer.h"

namespace Hayase
{
	class EmptyScene : public Scene, public Layer
	{
    public:
        EmptyScene() = default;
        EmptyScene(int windowWidth, int windowHeight);
        virtual ~EmptyScene();


    public:
        int Init() override;
        void CleanUp() override;

        int PreRender(float frame) override;
        int Render() override;
        int PostRender() override;

    public:
        virtual void OnAttach() override;
        virtual void OnDetach() override;
        virtual void Update() override;
        virtual void OnEvent(Event& e) override;

	};
}

#endif