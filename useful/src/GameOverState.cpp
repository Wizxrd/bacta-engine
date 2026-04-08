#include "GameOverState.hpp"

#include "GameConstants.hpp"
#include "MainMenuState.hpp"
#include "PlayState.hpp"
#include "StateManager.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <string>

GameOverState::GameOverState(StateContext& context, int finalScore)
: mCtx(context)
, mFinalScore(finalScore) {
	mSpaceWasDown =
		sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
		|| sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
	mMenuKeyWasDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M);
	mEscapeWasDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);

	if (context.hasMainFont()) {
		mLine1.emplace(context.mainFont(), "GAME OVER", 52u);
		mLine1->setFillColor(sf::Color(255, 120, 120));
		mLine1->setPosition({80.f, 140.f});

		mLine2.emplace(
			context.mainFont(),
			"Score: " + std::to_string(mFinalScore)
				+ "\nSPACE  retry     M  menu     ESC  quit",
			22u);
		mLine2->setFillColor(sf::Color(200, 205, 225));
		mLine2->setPosition({80.f, 240.f});
	}
}

void GameOverState::update(sf::Time deltaTime) {
	(void)deltaTime;

	const bool spaceDown =
		sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
		|| sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
	if (spaceDown && !mSpaceWasDown) {
		mCtx.pending.emplace_back([&ctx = mCtx](StateManager& sm) {
			if (!sm.empty()) {
				sm.pop();
			}
			sm.push(std::make_unique<PlayState>(ctx));
		});
	}
	mSpaceWasDown = spaceDown;

	const bool menuDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M);
	if (menuDown && !mMenuKeyWasDown) {
		mCtx.pending.emplace_back([&ctx = mCtx](StateManager& sm) {
			if (!sm.empty()) {
				sm.pop();
			}
			sm.push(std::make_unique<MainMenuState>(ctx));
		});
	}
	mMenuKeyWasDown = menuDown;

	const bool escDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
	if (escDown && !mEscapeWasDown) {
		mCtx.pending.emplace_back([&ctx = mCtx](StateManager&) {
			ctx.window.close();
		});
	}
	mEscapeWasDown = escDown;
}

void GameOverState::render(sf::RenderTarget& target) {
	sf::RectangleShape dim;
	dim.setSize({GameConstants::ViewWidth, GameConstants::ViewHeight});
	dim.setPosition({0.f, 0.f});
	dim.setFillColor(sf::Color(12, 14, 22, 210));
	target.draw(dim);

	if (mLine1) {
		target.draw(*mLine1);
	}
	if (mLine2) {
		target.draw(*mLine2);
	}
}
