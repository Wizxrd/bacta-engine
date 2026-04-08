#pragma once

#include "Resources.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include <functional>
#include <vector>

namespace Core{
	class StateManager;
	struct StateContext{
		StateContext(sf::RenderWindow& window, Resources& resources)
		: mWindow(window)
		, mResources(resources){
		}
		sf::RenderWindow& mWindow;
		Resources& mResources;
		std::vector<std::function<void(StateManager&)>> Pending;
	};
}