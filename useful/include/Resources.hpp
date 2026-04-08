#pragma once

#include "ResourceHolder.hpp"

#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <memory>
#include <vector>

namespace Textures {
	enum class Id { Player, Enemy, Bullet, Background };
}

namespace Fonts {
	enum class Id { Main };
}

namespace SoundBuffers {
	enum class Id { Shoot, EnemyHit, PlayerHurt, WeaponAk47Slst };
}

using TextureHolder = ResourceHolder<sf::Texture, Textures::Id>;
using FontHolder = ResourceHolder<sf::Font, Fonts::Id>;
using SoundBufferHolder = ResourceHolder<sf::SoundBuffer, SoundBuffers::Id>;

struct GameResources {
	TextureHolder textures;
	FontHolder fonts;
	SoundBufferHolder soundBuffers;

	std::vector<std::unique_ptr<sf::Texture>> handgunIdleFrames;
};
