#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

class Enemy {
	public:
		Enemy(sf::Vector2f position, int waveIndex);

		void update(sf::Time deltaTime, sf::Vector2f targetPosition);
		void takeHit(int damage);
		bool isDead() const { return mHp <= 0; }

		sf::Vector2f getPosition() const { return mPosition; }
		float getRadius() const { return mRadius; }
		int getHp() const { return mHp; }
		sf::Color getColor() const { return mColor; }

	private:
		sf::Vector2f mPosition;
		sf::Vector2f mVelocity{};
		float mRadius{22.f};
		int mHp{3};
		float mSpeed{95.f};
		sf::Color mColor{sf::Color(220, 90, 90)};
};
