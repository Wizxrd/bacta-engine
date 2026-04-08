#pragma once

#include "Core/State.hpp"
#include "Core/StateContext.hpp"
#include "UI/Button.hpp"

namespace States{
	class SoloMenu : public Core::State{
		public:
			SoloMenu(Core::StateContext& context);
			~SoloMenu();
			void HandleEvent(const sf::Event& event){}
			void Update(sf::Time deltaTime);
			void UpdateImGui(sf::Time deltaTime){}
			void Render(sf::RenderTarget& target);
		private:
			void initResources();
			void buildScene();
		private:
			sf::RectangleShape mBackground;
			Core::StateContext& mContext;

			std::vector<UI::Button> mButtons{};
	};
}