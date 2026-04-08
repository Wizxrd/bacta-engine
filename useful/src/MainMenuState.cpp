#include "MainMenuState.hpp"

#include "GameConstants.hpp"
#include "PlayState.hpp"
#include "StateManager.hpp"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Keyboard.hpp>

MainMenuState::MainMenuState(StateContext& context)
: mCtx(context) {
	mActionKeyWasDown =
		sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
		|| sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);
	mEscapeWasDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);

	if (context.hasMainFont()) {
		mTitle.emplace(context.mainFont(), "TOP-DOWN SHOOTA", 56u);
		mTitle->setFillColor(sf::Color(240, 245, 255));
		mTitle->setPosition({80.f, 120.f});

		mSubtitle.emplace(
			context.mainFont(), "SPACE / ENTER  play     ESC  quit", 22u);
		mSubtitle->setFillColor(sf::Color(180, 190, 215));
		mSubtitle->setPosition({80.f, 240.f});
	}
}

void MainMenuState::update(sf::Time deltaTime) {
	(void)deltaTime;

	const bool actionDown =
		sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space)
		|| sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Enter);

	if (actionDown && !mActionKeyWasDown) {
		mCtx.pending.emplace_back([&ctx = mCtx](StateManager& sm) {
			if (!sm.empty()) {
				sm.pop();
			}
			sm.push(std::make_unique<PlayState>(ctx));
		});
	}
	mActionKeyWasDown = actionDown;

	const bool escDown = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape);
	if (escDown && !mEscapeWasDown) {
		mCtx.pending.emplace_back([&ctx = mCtx](StateManager&) {
			ctx.window.close();
		});
	}
	mEscapeWasDown = escDown;
}

void MainMenuState::render(sf::RenderTarget& target) {
	sf::RectangleShape band;
	band.setSize({GameConstants::ViewWidth, 140.f});
	band.setPosition({0.f, 80.f});
	band.setFillColor(sf::Color(35, 38, 55));
	target.draw(band);

	if (mTitle) {
		target.draw(*mTitle);
	}
	if (mSubtitle) {
		target.draw(*mSubtitle);
	}

	if (!mCtx.hasMainFont()) {
		sf::RectangleShape hint;
		hint.setSize({420.f, 8.f});
		hint.setPosition({80.f, 320.f});
		hint.setFillColor(sf::Color(100, 200, 255));
		target.draw(hint);
	}
}
