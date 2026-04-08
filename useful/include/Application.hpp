#pragma once

#include "Resources.hpp"
#include "StateContext.hpp"
#include "StateManager.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>

class Application {
	public:
		Application();
		void run();

	private:
		void processEvents();
		void fixedUpdate(sf::Time deltaTime);
		void render();
		void loadResources();

		sf::RenderWindow mWindow;
		GameResources mResources;
		StateContext mContext;
		StateManager mStateManager;

		static const sf::Time TimePerFrame;
};
