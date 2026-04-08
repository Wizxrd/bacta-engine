#pragma once

#include "Resources.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include <functional>
#include <vector>

class StateManager;

struct StateContext {
	StateContext(sf::RenderWindow& window, GameResources& resources)
	: window(window)
	, resources(resources) {}

	sf::RenderWindow& window;
	GameResources& resources;

	std::vector<std::function<void(StateManager&)>> pending;

	bool hasMainFont() const {
		return resources.fonts.contains(Fonts::Id::Main);
	}

	const sf::Font& mainFont() const { return resources.fonts.get(Fonts::Id::Main); }
};
