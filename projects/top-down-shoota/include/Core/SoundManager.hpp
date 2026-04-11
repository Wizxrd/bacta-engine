#pragma once

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Audio/SoundSource.hpp>
#include <SFML/Audio/Sound.hpp>

#include <vector>
#include <memory>

namespace Core{
	class SoundManager{
		public:
			void Update();
			void PlayOnce(const sf::SoundBuffer& buffer);
		private:
			std::vector<sf::Sound> mSounds{};
	};
}