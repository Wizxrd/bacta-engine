#pragma once

#include "Bullet.hpp"
#include "Enemy.hpp"
#include "Player.hpp"
#include "State.hpp"
#include "StateContext.hpp"

#include <SFML/Graphics/Text.hpp>

#include <optional>
#include <random>
#include <vector>

class PlayState : public State {
	public:
		explicit PlayState(StateContext& context);

		void update(sf::Time deltaTime) override;
		void render(sf::RenderTarget& target) override;

	private:
		void spawnEnemy();
		void resolveCollisions();
		void cullOffscreenBullets();
		void renderHud(sf::RenderTarget& target);

		StateContext& mCtx;
		Player mPlayer;
		std::vector<Bullet> mBullets;
		std::vector<Enemy> mEnemies;

		std::mt19937 mRng;
		sf::Time mSpawnTimer{sf::Time::Zero};
		sf::Time mSpawnInterval{sf::seconds(1.15f)};

		int mScore{0};
		int mWave{0};
		int mKillsThisWave{0};
		static constexpr int KillsPerWave = 14;

		bool mGameOverSent{false};
		bool mBackToMenuKeysWereDown{false};

		std::optional<sf::Text> mHudText;
};
