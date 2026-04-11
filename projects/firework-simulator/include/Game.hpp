#pragma once

#include "Firework.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Time.hpp>

class Game{
	public:
		void ProcessEvent(sf::RenderWindow& window, const sf::Event& event);
		void UpdateImGui(sf::Time deltaTime);
		void Update(sf::Time deltaTime);
		void Render(sf::RenderTarget& target);
	private:
		std::vector<Firework> mFireworks;
};