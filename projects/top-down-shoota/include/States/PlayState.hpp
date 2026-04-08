#pragma once

#include "Core/State.hpp"
#include "Core/StateContext.hpp"
#include "Core/Soldier.hpp"
#include "Core/Player.hpp"

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
			void handlePlayerInput(sf::Time deltaTime);
			void handlePlayerMouse();
			void updateCameraLerp(sf::Time deltaTime);
			void updateZoom(sf::Time deltaTime);
		private:
			sf::RectangleShape mBackground;
			sf::View mWorldView;
			float mZoom = 1000.f;
			float mTargetZoom = 1000.f;
			float mCameraSmooting = 5.f;
			float mZoomSmoothing = 2.5f;
			Core::StateContext& mContext;
			Core::Player mPlayer;
			Core::Soldier mSoldier;
	};
}