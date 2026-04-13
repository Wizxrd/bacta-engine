#pragma once

#include "Resources.hpp"
#include "Core/InputManager.hpp"
#include "Core/SoundManager.hpp"

#include <SFML/Graphics/window.hpp>

#include <functional>
#include <vector>

namespace Core{
	class StateManager;
	struct StateContext{
		StateContext(sf::window& window, sf::View& view, Resources& resources, InputManager& input, SoundManager& sound)
		: mWindow(window)
		, mView(view)
		, mResources(resources)
		, mInputManager(input)
		, mSoundManager(sound){
		}
		sf::window& mWindow;
		sf::View mView;
		Resources& mResources;
		InputManager& mInputManager;
		SoundManager& mSoundManager;
		std::vector<std::function<void(StateManager&)>> Pending;
	};
}