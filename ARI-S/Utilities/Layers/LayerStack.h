#ifndef LAYERSTACK_H
#define LAYERSTACK_H

#include "Layer.h"

namespace ARIS
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
		std::vector<Layer*>::reverse_iterator rbegin() { return m_Layers.rbegin(); }
		std::vector<Layer*>::reverse_iterator rend() { return m_Layers.rend(); }

		std::vector<Layer*>::const_iterator begin() const { return m_Layers.begin(); }
		std::vector<Layer*>::const_iterator end() const { return m_Layers.end(); }
		std::vector<Layer*>::const_reverse_iterator rbegin() const { return m_Layers.rbegin(); }
		std::vector<Layer*>::const_reverse_iterator rend() const { return m_Layers.rend(); }
	private:
		std::vector<Layer*> m_Layers;
		unsigned m_LayerInsertIdx = 0;
	};
}

#endif