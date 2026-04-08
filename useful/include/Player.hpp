#pragma once

#include "Resources.hpp"
#include "SpriteAnimation.hpp"

#include <SFML/Audio/Sound.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <memory>
#include <vector>

class Bullet;

class Player {
	public:
		Player(sf::Vector2f startPosition, GameResources& resources);

		void update(sf::Time deltaTime, const sf::RenderWindow& window);
		void tryFire(std::vector<Bullet>& bullets, const sf::RenderWindow& window);
		void takeDamage(int amount);
		void render(sf::RenderTarget& target) const;

		sf::Vector2f getPosition() const { return mPosition; }
		float getRadius() const { return mRadius; }
		bool isDead() const { return mHp <= 0; }
		int getHp() const { return mHp; }
		int getMaxHp() const { return mMaxHp; }

	private:
		sf::Vector2f computeAimDir(const sf::RenderWindow& window) const;

		GameResources& mResources;
		SpriteAnimation mHandgunAnim;
		std::unique_ptr<sf::Sound> mGunSound;

		sf::Vector2f mPosition;
		float mRadius{18.f};
		float mSpeed{340.f};
		int mHp{100};
		int mMaxHp{100};
		sf::Time mFireCooldown{sf::Time::Zero};
		sf::Time mContactDamageCooldown{sf::Time::Zero};
};
