#pragma once

#include "Core/State.hpp"
#include "Core/StateContext.hpp"
#include "Core/Soldier.hpp"
#include "Core/Player.hpp"
#include "Core/Camera.hpp"

#include <SFML/Graphics/RectangleShape.hpp>

/*
okay here is our play state, this will be where we finally control and animate the player sprite
*/

namespace States{
	class PlayState : public Core::State{
		public:
			PlayState(Core::StateContext& context);
			~PlayState();
			void HandleEvent(const sf::Event& event);
			void Update(sf::Time deltaTime);
			void UpdateImGui(sf::Time deltaTime);
			void Render(sf::RenderTarget& target);
		private:
			void initResources();
			void buildScene();
		private:
			sf::RectangleShape mBackground;
			Core::Camera mCamera;
			Core::StateContext& mContext;
			Core::Soldier mSoldier;
			Core::Player mPlayer;
	};
}