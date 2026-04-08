#include "Player.hpp"

#include "Bullet.hpp"
#include "GameConstants.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <algorithm>
#include <cmath>

namespace {
	float length(sf::Vector2f v) {
		return std::sqrt(v.x * v.x + v.y * v.y);
	}

	constexpr float RadToDeg = 180.f / 3.14159265f;
}

Player::Player(sf::Vector2f startPosition, GameResources& resources)
: mResources(resources)
, mPosition(startPosition) {
	std::vector<const sf::Texture*> frames;
	frames.reserve(resources.handgunIdleFrames.size());
	for (const auto& tex : resources.handgunIdleFrames) {
		frames.push_back(tex.get());
	}
	mHandgunAnim.setFrames(std::move(frames));
	mHandgunAnim.setFrameDuration(sf::seconds(0.025f));
	mHandgunAnim.setTargetWidth(56.f);

}

sf::Vector2f Player::computeAimDir(const sf::RenderWindow& window) const {
	const sf::Vector2i mousePx = sf::Mouse::getPosition(window);
	const sf::Vector2f aim{
		static_cast<float>(mousePx.x), static_cast<float>(mousePx.y)};
	sf::Vector2f dir = aim - mPosition;
	const float len = length(dir);
	if (len < 1e-3f) {
		return {0.f, -1.f};
	}
	return {dir.x / len, dir.y / len};
}

void Player::update(sf::Time deltaTime, const sf::RenderWindow& window) {
	sf::Vector2f move{0.f, 0.f};
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)
		|| sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
		move.y -= 1.f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)
		|| sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
		move.y += 1.f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)
		|| sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
		move.x -= 1.f;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)
		|| sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
		move.x += 1.f;
	}

	if (move.x != 0.f || move.y != 0.f) {
		const float len = length(move);
		move /= len;
	}

	mPosition += move * mSpeed * deltaTime.asSeconds();

	const float m = GameConstants::Margin + mRadius;
	mPosition.x = std::clamp(
		mPosition.x, m, GameConstants::ViewWidth - m);
	mPosition.y = std::clamp(
		mPosition.y, m, GameConstants::ViewHeight - m);

	if (mFireCooldown > sf::Time::Zero) {
		mFireCooldown -= deltaTime;
	}
	if (mContactDamageCooldown > sf::Time::Zero) {
		mContactDamageCooldown -= deltaTime;
	}

	const sf::Vector2f aimDir = computeAimDir(window);
	const float aimDeg = std::atan2(aimDir.y, aimDir.x) * RadToDeg;
	mHandgunAnim.setPosition(mPosition);
	mHandgunAnim.setRotation(aimDeg);
	mHandgunAnim.update(deltaTime);
}

void Player::tryFire(std::vector<Bullet>& bullets, const sf::RenderWindow& window) {
	const bool wantFire = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)
		|| sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space);
	if (!wantFire || mFireCooldown > sf::Time::Zero) {
		return;
	}

	const sf::Vector2f dir = computeAimDir(window);
	bullets.emplace_back(mPosition, dir, 780.f);
	mFireCooldown = sf::seconds(0.11f);

	if (mResources.soundBuffers.contains(SoundBuffers::Id::WeaponAk47Slst)) {
		if (!mGunSound) {
			mGunSound = std::make_unique<sf::Sound>(mResources.soundBuffers.get(SoundBuffers::Id::WeaponAk47Slst));
		}
		mGunSound->play();
	}
}

void Player::takeDamage(int amount) {
	if (mContactDamageCooldown > sf::Time::Zero) {
		return;
	}
	mHp -= amount;
	mContactDamageCooldown = sf::seconds(0.45f);
}

void Player::render(sf::RenderTarget& target) const {
	if (mHandgunAnim.hasFrames()) {
		sf::CircleShape hit(mRadius);
		hit.setOrigin({mRadius, mRadius});
		hit.setPosition(mPosition);
		hit.setFillColor(sf::Color(0, 0, 0, 0));
		hit.setOutlineColor(sf::Color(120, 200, 255, 140));
		hit.setOutlineThickness(2.f);
		target.draw(hit);
		mHandgunAnim.draw(target);
		return;
	}

	sf::CircleShape body(mRadius);
	body.setOrigin({mRadius, mRadius});
	body.setPosition(mPosition);
	body.setFillColor(sf::Color(120, 200, 255));
	body.setOutlineColor(sf::Color(220, 245, 255));
	body.setOutlineThickness(2.f);
	target.draw(body);
}
