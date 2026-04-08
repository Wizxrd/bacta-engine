#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <optional>
#include <vector>

class SpriteAnimation {
	public:
		void clear();
		void setFrames(std::vector<const sf::Texture*> frames);
		void setFrameDuration(sf::Time duration);
		void setTargetWidth(float pixels);
		void update(sf::Time deltaTime);

		void setPosition(sf::Vector2f position);
		void setRotation(float angleDegrees);

		void draw(sf::RenderTarget& target, sf::RenderStates states = {}) const;

		bool hasFrames() const { return !mFrames.empty(); }

	private:
		void applyFrame(std::size_t index);

		std::vector<const sf::Texture*> mFrames;
		std::optional<sf::Sprite> mSprite;
		sf::Time mFrameDuration{sf::seconds(0.025f)};
		sf::Time mAccum{sf::Time::Zero};
		std::size_t mFrameIndex{0};
		float mTargetWidth{0.f};
};
