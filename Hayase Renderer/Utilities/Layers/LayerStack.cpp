#include <hyspch.h>

#include "LayerStack.h"

namespace Hayase
{
	LayerStack::LayerStack()
	{
	}

	LayerStack::~LayerStack()
	{
		for (Layer* l : m_Layers)
		{
			delete l;
		}
	}

	void LayerStack::PushLayer(Layer* l)
	{
		m_Layers.emplace(m_Layers.begin() + m_LayerInsertIdx, l);
		++m_LayerInsertIdx;
	}

	void LayerStack::PopLayer(Layer* l)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), l);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
			m_LayerInsertIdx--;
		}
	}

	void LayerStack::PushOverlay(Layer* o)
	{
		m_Layers.emplace_back(o);
	}

	void LayerStack::PopOverlay(Layer* o)
	{
		auto it = std::find(m_Layers.begin(), m_Layers.end(), o);
		if (it != m_Layers.end())
		{
			m_Layers.erase(it);
		}
	}

}