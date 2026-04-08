#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

class Bullet {
	public:
		Bullet(sf::Vector2f position, sf::Vector2f direction, float speed);

		void update(sf::Time deltaTime);
		bool isDead() const;
		void markDead();

		sf::Vector2f getPosition() const { return mPosition; }
		float getRadius() const { return mRadius; }
		sf::Color getColor() const { return mColor; }

	private:
		sf::Vector2f mPosition;
		sf::Vector2f mVelocity;
		float mRadius{5.f};
		sf::Time mLifetime{sf::Time::Zero};
		sf::Time mMaxLifetime;
		sf::Color mColor{sf::Color(255, 230, 120)};
		bool mDead{false};
};
