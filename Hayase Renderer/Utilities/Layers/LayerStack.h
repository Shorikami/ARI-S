#ifndef LAYERSTACK_H
#define LAYERSTACK_H

#include "Layer.h"

namespace Hayase
{
	class LayerStack
	{
	public:
		LayerStack();
		~LayerStack();

		void PushLayer(Layer* l);
		void PopLayer(Layer* l);

		void PushOverlay(Layer* o);
		void PopOverlay(Layer* o);

		std::vector<Layer*>::iterator begin() { return m_Layers.begin(); }
		std::vector<Layer*>::iterator end() { return m_Layers.end(); }
	private:
		std::vector<Layer*> m_Layers;
		std::vector<Layer*>::iterator m_LayerInsert;
	};
}

#endif