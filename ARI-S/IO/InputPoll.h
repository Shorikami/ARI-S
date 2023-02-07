#ifndef INPUT_H
#define INPUT_H

#include <glm.hpp>

#include "KeyCodes.h"
#include "MouseCodes.h"

namespace ARIS
{
	class InputPoll 
	{
	public:
		static bool IsKeyPressed(KeyCode key);
		static bool IsMouseButtonPressed(MouseCode button);
		static glm::vec2 GetMousePosition();
	};
}

#endif