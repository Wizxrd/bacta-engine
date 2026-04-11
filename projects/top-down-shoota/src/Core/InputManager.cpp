#include "Core/InputManager.hpp"

namespace Core{

	void InputManager::Update(){
		mPreviousKeys = mCurrentKeys;
		for (int i = 0; i < sf::Keyboard::ScancodeCount; i++){
			auto scancode = static_cast<sf::Keyboard::Scancode>(i);
			mCurrentKeys[i] = sf::Keyboard::isKeyPressed(scancode);
			mPressedKeys[i] = mCurrentKeys[i] && !mPreviousKeys[i];
			mReleasedKeys[i] = !mCurrentKeys[i] && mPreviousKeys[i];
		}

		mPreviousMouse = mCurrentMouse;
		for (int i = 0; i < sf::Mouse::ButtonCount; i++){
			auto button = static_cast<sf::Mouse::Button>(i);
			mCurrentMouse[i] = sf::Mouse::isButtonPressed(button);
			mPressedMouse[i] = mCurrentMouse[i] && !mPreviousMouse[i];
			mReleasedMouse[i] = !mCurrentMouse[i] && mPreviousMouse[i];
		}
	}
	bool InputManager::IsKeyPressed(sf::Keyboard::Scancode scancode) const{
		return mPressedKeys[static_cast<std::size_t>(scancode)];
	}

	bool InputManager::IsKeyHeld(sf::Keyboard::Scancode scancode) const{
		return mCurrentKeys[static_cast<std::size_t>(scancode)];
	}

	bool InputManager::WasKeyReleased(sf::Keyboard::Scancode scancode) const{
		return mReleasedKeys[static_cast<std::size_t>(scancode)];
	}

	bool InputManager::IsMousePressed(sf::Mouse::Button button) const{
		return mPressedMouse[static_cast<std::size_t>(button)];
	}

	bool InputManager::IsMouseHeld(sf::Mouse::Button button) const{
		return mCurrentMouse[static_cast<std::size_t>(button)];
	}
}