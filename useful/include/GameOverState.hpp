#pragma once

#include "State.hpp"
#include "StateContext.hpp"

#include <SFML/Graphics/Text.hpp>

#include <optional>

class GameOverState : public State {
	public:
		GameOverState(StateContext& context, int finalScore);

		void update(sf::Time deltaTime) override;
		void render(sf::RenderTarget& target) override;

	private:
		StateContext& mCtx;
		int mFinalScore;

		bool mSpaceWasDown{false};
		bool mMenuKeyWasDown{false};
		bool mEscapeWasDown{false};

		std::optional<sf::Text> mLine1;
		std::optional<sf::Text> mLine2;
};
