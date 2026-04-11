#pragma once

#include "Animation.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>

#include <memory>

#include <iostream>

namespace Core{
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