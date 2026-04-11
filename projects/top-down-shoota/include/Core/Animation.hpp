#pragma once

#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include <vector>

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
}