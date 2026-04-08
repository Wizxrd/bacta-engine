#pragma once

#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>

namespace Core{
	class State{
		public:
			virtual ~State() = default;
			virtual void HandleEvent(const sf::Event& event) = 0;
			virtual void Update(sf::Time deltaTime) = 0;
			virtual void UpdateImGui(sf::Time deltaTime) = 0;
			virtual void Render(sf::RenderTarget& target) = 0;
	};
}