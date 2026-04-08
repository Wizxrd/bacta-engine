#pragma once

#include "Core/State.hpp"
#include "Core/StateContext.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>

#include <memory>

namespace States{
	class StartMenu : public Core::State{
		public:
			StartMenu(Core::StateContext& context);
			~StartMenu() = default;
			void HandleEvent(const sf::Event& event){}
			void Update(sf::Time deltaTime);
			void UpdateImGui(sf::Time deltaTime){}
			void Render(sf::RenderTarget& target);
		private:
			void initResources();
			void buildScene();
			void animateText(sf::Time deltaTime);
		private:
			sf::RectangleShape mBackground;
			Core::StateContext& mContext;
			std::unique_ptr<sf::Text> mText;
			int mFontSizeDirection = 1;
			float mMaxFontSize = 26;
			float mMinFontSize = 24;
			float mElapsedTime = 0.f;
	};
}