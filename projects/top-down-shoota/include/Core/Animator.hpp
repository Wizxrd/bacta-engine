#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Time.hpp>

#include <vector>
#include <memory>

#include <iostream>

namespace Core{

	struct Animation{

		Animation(sf::Texture& spriteSheet, int frameCount) : SpriteSheet(spriteSheet), FrameCount(frameCount){
			const sf::Vector2u size = spriteSheet.getSize();
			const int width = size.x / frameCount;
			const int height = size.y;
			Frames.reserve(static_cast<std::size_t>(frameCount));
			for (int i = 0; i < frameCount; i++){
				Frames.push_back(sf::IntRect({i * width, 0},{width, height}));
			}
		}

		sf::Texture& SpriteSheet;
		std::vector<sf::IntRect> Frames{};
		int FrameCount = 0;
		float SecondsPerFrame = 0.05f;
		bool Loop = false;
	};

	class Animator{
		public:
			void Play(const Animation& animation);
			void Update(sf::Time deltaTime);
			void Render(sf::RenderTarget& target);
			void SetTarget(sf::Sprite& sprite);
			bool IsComplete() const;
			const Animation& GetCurrent() const;
			std::size_t mFrame{};
		private:
			sf::Sprite* mTarget = nullptr;
			const Animation* mAnimation = nullptr;
			//std::size_t mFrame{};
			float mElapsed = 0.f;
			bool mComplete = false;
	};
}