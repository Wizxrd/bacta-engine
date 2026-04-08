#include "Bullet.hpp"

#include <cmath>

namespace {
	float length(sf::Vector2f v) {
		return std::sqrt(v.x * v.x + v.y * v.y);
	}

	sf::Vector2f normalize(sf::Vector2f v) {
		const float len = length(v);
		if (len < 1e-5f) {
			return {0.f, -1.f};
		}
		return {v.x / len, v.y / len};
	}
}

Bullet::Bullet(sf::Vector2f position, sf::Vector2f direction, float speed)
: mPosition(position)
, mVelocity(normalize(direction) * speed)
, mMaxLifetime(sf::seconds(1.35f)) {}

void Bullet::update(sf::Time deltaTime) {
	mPosition += mVelocity * deltaTime.asSeconds();
	mLifetime += deltaTime;
	if (mLifetime >= mMaxLifetime) {
		mDead = true;
	}
}

bool Bullet::isDead() const {
	return mDead;
}

void Bullet::markDead() {
	mDead = true;
}
