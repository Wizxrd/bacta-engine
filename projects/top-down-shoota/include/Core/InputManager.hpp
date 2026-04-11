#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <array>

namespace Core{
	class InputManager{
		public:
			void Update();
			bool IsKeyPressed(sf::Keyboard::Scancode scancode) const;
			bool IsKeyHeld(sf::Keyboard::Scancode scancode) const;
			bool WasKeyReleased(sf::Keyboard::Scancode scancode) const;
			bool IsMousePressed(sf::Mouse::Button button) const;
			bool IsMouseHeld(sf::Mouse::Button button) const;
		private:
			std::array<bool, sf::Keyboard::ScancodeCount> mPreviousKeys{};
			std::array<bool, sf::Keyboard::ScancodeCount> mCurrentKeys{};
			std::array<bool, sf::Keyboard::ScancodeCount> mPressedKeys{};
			std::array<bool, sf::Keyboard::ScancodeCount> mReleasedKeys{};

			std::array<bool, sf::Mouse::ButtonCount> mPreviousMouse{};
			std::array<bool, sf::Mouse::ButtonCount> mCurrentMouse{};
			std::array<bool, sf::Mouse::ButtonCount> mPressedMouse{};
			std::array<bool, sf::Mouse::ButtonCount> mReleasedMouse{};
	};
}