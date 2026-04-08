#pragma once

#include "State.hpp"
#include "StateContext.hpp"

#include <SFML/Graphics/Text.hpp>

#include <optional>

class MainMenuState : public State {
	public:
		explicit MainMenuState(StateContext& context);

		void update(sf::Time deltaTime) override;
		void render(sf::RenderTarget& target) override;

	private:
		StateContext& mCtx;
		bool mActionKeyWasDown{false};
		bool mEscapeWasDown{false};

		std::optional<sf::Text> mTitle;
		std::optional<sf::Text> mSubtitle;
};
