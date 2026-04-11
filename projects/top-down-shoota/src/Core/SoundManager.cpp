#include "Core/SoundManager.hpp"

#include <algorithm>
#include <iostream>

namespace Core{
	void SoundManager::Update(){
		std::cout << mSounds.size() << std::endl;
	}

	void SoundManager::PlayOnce(const sf::SoundBuffer& buffer){
		mSounds.erase(
			std::remove_if(mSounds.begin(), mSounds.end(),
				[](const sf::Sound& sound){
					return sound.getStatus() == sf::SoundSource::Status::Stopped;
				}
			), mSounds.end()
		);
		mSounds.emplace_back(buffer);
		mSounds.back().play();
	}
}