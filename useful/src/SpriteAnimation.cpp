#include "SpriteAnimation.hpp"

#include <SFML/System/Angle.hpp>

void SpriteAnimation::clear() {
	mFrames.clear();
	mSprite.reset();
	mFrameIndex = 0;
	mAccum = sf::Time::Zero;
}

void SpriteAnimation::setFrames(std::vector<const sf::Texture*> frames) {
	mFrames = std::move(frames);
	mFrameIndex = 0;
	mAccum = sf::Time::Zero;
	if (mFrames.empty()) {
		mSprite.reset();
	} else {
		applyFrame(0);
	}
}

void SpriteAnimation::setFrameDuration(sf::Time duration) {
	mFrameDuration = duration;
}

void SpriteAnimation::setTargetWidth(float pixels) {
	mTargetWidth = pixels;
	if (!mFrames.empty()) {
		applyFrame(mFrameIndex);
	}
}

void SpriteAnimation::applyFrame(std::size_t index) {
	if (index >= mFrames.size()) {
		return;
	}
	const sf::Texture& tex = *mFrames[index];
	if (!mSprite) {
		mSprite.emplace(tex);
	} else {
		mSprite->setTexture(tex);
	}
	const sf::Vector2u sz = tex.getSize();
	mSprite->setOrigin({sz.x * 0.5f, sz.y * 0.5f});
	if (mTargetWidth > 0.f && sz.x > 0u) {
		const float s = mTargetWidth / static_cast<float>(sz.x);
		mSprite->setScale({s, s});
	} else {
		mSprite->setScale({1.f, 1.f});
	}
}

void SpriteAnimation::update(sf::Time deltaTime) {
	if (mFrames.empty() || mFrameDuration <= sf::Time::Zero) {
		return;
	}
	mAccum += deltaTime;
	while (mAccum >= mFrameDuration) {
		mAccum -= mFrameDuration;
		mFrameIndex = (mFrameIndex + 1) % mFrames.size();
		applyFrame(mFrameIndex);
	}
}

void SpriteAnimation::setPosition(sf::Vector2f position) {
	if (mSprite) {
		mSprite->setPosition(position);
	}
}

void SpriteAnimation::setRotation(float angleDegrees) {
	if (mSprite) {
		mSprite->setRotation(sf::degrees(angleDegrees));
	}
}

void SpriteAnimation::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	if (mSprite) {
		target.draw(*mSprite, states);
	}
}
