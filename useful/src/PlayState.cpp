#include "PlayState.hpp"

#include "GameConstants.hpp"
#include "GameOverState.hpp"
#include "MainMenuState.hpp"
#include "StateManager.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Window/Keyboard.hpp>

#include <algorithm>
#include <cmath>
#include <string>

namespace {
	float distance(sf::Vector2f a, sf::Vector2f b) {
		const float dx = a.x - b.x;
		const float dy = a.y - b.y;
		return std::sqrt(dx * dx + dy * dy);
	}

	void requestGameOver(StateContext& ctx, int finalScore) {
		ctx.pending.emplace_back([&ctx, finalScore](StateManager& sm) {
			if (!sm.empty()) {
				sm.pop();
			}
			sm.push(std::make_unique<GameOverState>(ctx, finalScore));
		});
	}
}

PlayState::PlayState(StateContext& context)
: mCtx(context)
, mPlayer(
		  sf::Vector2f{
			  GameConstants::ViewWidth * 0.5f,
			  GameConstants::ViewHeight * 0.5f},
		  context.resources)
, mRng(std::random_device{}()) {
	mBackToMenuKeysWereDown =
		sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)
		|| sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M);

	if (context.hasMainFont()) {
		mHudText.emplace(context.mainFont(), "", 18u);
		mHudText->setFillColor(sf::Color(235, 240, 255));
		mHudText->setPosition({18.f, 14.f});
	}
}

void PlayState::spawnEnemy() {
	std::uniform_int_distribution<int> edgeDist(0, 3);
	std::uniform_real_distribution<float> along(
		GameConstants::Margin * 2.f,
		GameConstants::ViewWidth - GameConstants::Margin * 2.f);
	std::uniform_real_distribution<float> alongY(
		GameConstants::Margin * 2.f,
		GameConstants::ViewHeight - GameConstants::Margin * 2.f);

	const int edge = edgeDist(mRng);
	sf::Vector2f pos{0.f, 0.f};
	switch (edge) {
		case 0:
			pos = {along(mRng), GameConstants::Margin};
			break;
		case 1:
			pos = {along(mRng), GameConstants::ViewHeight - GameConstants::Margin};
			break;
		case 2:
			pos = {GameConstants::Margin, alongY(mRng)};
			break;
		default:
			pos = {GameConstants::ViewWidth - GameConstants::Margin, alongY(mRng)};
			break;
	}
	mEnemies.emplace_back(pos, mWave);
}

void PlayState::cullOffscreenBullets() {
	const float pad = 60.f;
	for (auto& b : mBullets) {
		const sf::Vector2f p = b.getPosition();
		if (p.x < -pad || p.y < -pad || p.x > GameConstants::ViewWidth + pad
			|| p.y > GameConstants::ViewHeight + pad) {
			b.markDead();
		}
	}
}

void PlayState::resolveCollisions() {
	const sf::Vector2f playerPos = mPlayer.getPosition();
	const float pr = mPlayer.getRadius();

	for (auto& e : mEnemies) {
		if (e.isDead()) {
			continue;
		}
		const float dist = distance(playerPos, e.getPosition());
		if (dist < pr + e.getRadius()) {
			mPlayer.takeDamage(14);
		}
	}

	for (auto& b : mBullets) {
		if (b.isDead()) {
			continue;
		}
		for (auto& e : mEnemies) {
			if (e.isDead()) {
				continue;
			}
			const float dist = distance(b.getPosition(), e.getPosition());
			if (dist < b.getRadius() + e.getRadius()) {
				e.takeHit(1);
				b.markDead();
				if (e.isDead()) {
					mScore += 10 + mWave;
					++mKillsThisWave;
					if (mKillsThisWave >= KillsPerWave) {
						mKillsThisWave = 0;
						++mWave;
						const float sec = 1.15f - static_cast<float>(mWave) * 0.04f;
						mSpawnInterval = sf::seconds(std::max(0.32f, sec));
					}
				}
				break;
			}
		}
	}
}

void PlayState::update(sf::Time deltaTime) {
	const bool backToMenuDown =
		sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape)
		|| sf::Keyboard::isKeyPressed(sf::Keyboard::Key::M);
	if (backToMenuDown && !mBackToMenuKeysWereDown) {
		mCtx.pending.emplace_back([&ctx = mCtx](StateManager& sm) {
			if (!sm.empty()) {
				sm.pop();
			}
			sm.push(std::make_unique<MainMenuState>(ctx));
		});
		mBackToMenuKeysWereDown = true;
		return;
	}
	mBackToMenuKeysWereDown = backToMenuDown;

	if (mGameOverSent) {
		return;
	}

	mSpawnTimer += deltaTime;
	while (mSpawnTimer >= mSpawnInterval) {
		mSpawnTimer -= mSpawnInterval;
		spawnEnemy();
	}

	mPlayer.update(deltaTime, mCtx.window);
	mPlayer.tryFire(mBullets, mCtx.window);

	for (auto& b : mBullets) {
		b.update(deltaTime);
	}
	cullOffscreenBullets();

	for (auto& e : mEnemies) {
		e.update(deltaTime, mPlayer.getPosition());
	}

	resolveCollisions();

	mBullets.erase(
		std::remove_if(
			mBullets.begin(),
			mBullets.end(),
			[](const Bullet& b) { return b.isDead(); }),
		mBullets.end());

	mEnemies.erase(
		std::remove_if(
			mEnemies.begin(),
			mEnemies.end(),
			[](const Enemy& e) { return e.isDead(); }),
		mEnemies.end());

	if (mPlayer.isDead()) {
		if (!mGameOverSent) {
			mGameOverSent = true;
			requestGameOver(mCtx, mScore);
		}
		return;
	}
}

void PlayState::renderHud(sf::RenderTarget& target) {
	const float maxW = 220.f;
	const float h = 14.f;
	const float x = 18.f;
	const float y = GameConstants::ViewHeight - 36.f;

	sf::RectangleShape back({maxW, h});
	back.setPosition({x, y});
	back.setFillColor(sf::Color(40, 44, 58));
	back.setOutlineColor(sf::Color(90, 96, 120));
	back.setOutlineThickness(1.f);
	target.draw(back);

	const float ratio = static_cast<float>(mPlayer.getHp())
		/ static_cast<float>(std::max(1, mPlayer.getMaxHp()));
	sf::RectangleShape fill({maxW * ratio, h - 4.f});
	fill.setPosition({x + 2.f, y + 2.f});
	fill.setFillColor(sf::Color(80, 220, 120));
	target.draw(fill);

	if (mHudText) {
		mHudText->setString(
			"Score " + std::to_string(mScore) + "   Wave " + std::to_string(mWave + 1)
			+ "     ESC / M  main menu");
		target.draw(*mHudText);
	}
}

void PlayState::render(sf::RenderTarget& target) {
	sf::RectangleShape floor;
	floor.setSize(
		{GameConstants::ViewWidth - 2.f, GameConstants::ViewHeight - 2.f});
	floor.setPosition({1.f, 1.f});
	floor.setFillColor(sf::Color(28, 30, 42));
	floor.setOutlineColor(sf::Color(55, 60, 80));
	floor.setOutlineThickness(1.f);
	target.draw(floor);

	for (const auto& e : mEnemies) {
		sf::CircleShape c(e.getRadius());
		c.setOrigin({e.getRadius(), e.getRadius()});
		c.setPosition(e.getPosition());
		c.setFillColor(e.getColor());
		c.setOutlineColor(sf::Color(40, 40, 40));
		c.setOutlineThickness(1.f);
		target.draw(c);
	}

	for (const auto& b : mBullets) {
		sf::CircleShape c(b.getRadius());
		c.setOrigin({b.getRadius(), b.getRadius()});
		c.setPosition(b.getPosition());
		c.setFillColor(b.getColor());
		target.draw(c);
	}

	mPlayer.render(target);
	renderHud(target);
}
