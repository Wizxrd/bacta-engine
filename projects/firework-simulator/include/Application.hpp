#pragma once

#include "Game.hpp"

#include <SFML/Graphics/RenderWindow.hpp>

#include <vector>

class Application{
	public:
		Application();
		void Run();
	private:
		void processEvent();
		void update(sf::Time deltaTime);
		void updateImGui(sf::Time deltaTime);
		void render();
	private:
		sf::RenderWindow mWindow;
		Game mGame;
		float FPS = 120.f;
		static const sf::Time TimePerFrame;
};