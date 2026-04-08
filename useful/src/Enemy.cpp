#include "Enemy.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace {
	float length(sf::Vector2f v) {
		return std::sqrt(v.x * v.x + v.y * v.y);
	}
}

Enemy::Enemy(sf::Vector2f position, int waveIndex)
: mPosition(position) {
	mSpeed = 88.f + static_cast<float>(waveIndex) * 4.5f;
	mHp = 3 + waveIndex / 4;
	mRadius = 20.f + static_cast<float>(waveIndex % 8) * 0.5f;
	const int shade = 200 - (waveIndex * 3 % 80);
	mColor = sf::Color(
		static_cast<std::uint8_t>(std::min(255, 180 + waveIndex * 2)),
		static_cast<std::uint8_t>(std::max(40, shade)),
		static_cast<std::uint8_t>(std::max(40, 100 - waveIndex)));
}

void Enemy::update(sf::Time deltaTime, sf::Vector2f targetPosition) {
	sf::Vector2f to = targetPosition - mPosition;
	const float len = length(to);
	if (len > 1e-3f) {
		mVelocity = (to / len) * mSpeed;
	}
	mPosition += mVelocity * deltaTime.asSeconds();
}

void Enemy::takeHit(int damage) {
	mHp -= damage;
}
