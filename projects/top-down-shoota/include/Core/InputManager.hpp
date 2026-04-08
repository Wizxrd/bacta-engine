#pragma once

namespace Core{
	class InputManager{
		public:
			bool IsKeyDown();
			bool WasKeyDown();
			bool IsKeyReleased();
			bool IsMouseDown();
			bool WasMouseDown();
	};
}