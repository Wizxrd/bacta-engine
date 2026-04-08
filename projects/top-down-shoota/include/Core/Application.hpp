#pragma once

#include "Core/Resources.hpp"
#include "Core/StateContext.hpp"
#include "Core/StateManager.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>

#include <memory>

namespace Core{
	class Application{
		public:
			Application();
			void Run();
		private:
			void processEvents();
			void update(sf::Time deltaTime);
			void render();
		private:
			sf::RenderWindow mWindow;
			StateContext mContext;
			Resources mResources;
			StateManager mStateManager;

			static const sf::Time TimePerFrame;
	};
}