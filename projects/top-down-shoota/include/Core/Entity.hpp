#pragma once

#include <SFML/System/Vector2.hpp>
#include <SFML/System/Time.hpp>

namespace Core{
	class Entity{
		public:
			virtual void SetVelocity(sf::Vector2f velocity);
			virtual sf::Vector2f GetVelocity() const;
			virtual void Accelerate(sf::Vector2f velocity);
		private:
			sf::Vector2f mVelocity = {0.f, 0.f};
	};
}