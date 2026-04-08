#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>

class State {
	public:
		virtual ~State() = default;
		virtual void update(sf::Time deltaTime) = 0;
		virtual void render(sf::RenderTarget& target) = 0;
};
