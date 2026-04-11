#pragma once

#include "Core/ResourceManager.hpp"

#include <SFML/Audio/Sound.hpp>

#include <memory>
#include <vector>

namespace Core::Textures{
	enum class Id{
		// player
			// feet
				Player_Feet_Idle,
				Player_Feet_Walk,
				Player_Feet_Run,
				Player_Feet_Strafe_Left,
				Player_Feet_Strafe_Right,
			// flashlight
				Player_Flashlight_Idle,
				Player_Flashlight_Melee,
				Player_Flashlight_Move,
			// handgun
				Player_Handgun_Idle,
				Player_Handgun_Melee,
				Player_Handgun_Move,
				Player_Handgun_Reload,
				Player_Handgun_Shoot,
			// knife
				Player_Knife_Idle,
				Player_Knife_Melee,
				Player_Knife_Move,
			// rifle
				Player_Rifle_Idle,
				Player_Rifle_Melee,
				Player_Rifle_Move,
				Player_Rifle_Reload,
				Player_Rifle_Shoot,
			// shotgun
				Player_Shotgun_Idle,
				Player_Shotgun_Melee,
				Player_Shotgun_Move,
				Player_Shotgun_Reload,
				Player_Shotgun_Shoot,
		// tiles
			Tile_Stone,
		// ui
			Soldier_Background,
	};
}

namespace Core::Fonts{
	enum class Id{
		Default,
		Header,
		Bold
	};
}

namespace Core::SoundBuffers{
	enum class Id{
		Handgun_Fire,
		Handgun_Dryfire,
		Handgun_Reload,

		Rifle_Fire,
		Rifle_Dryfire,
		Rifle_Reload,

		Shotgun_Fire,
		Shotgun_Dryfire,
		Shotgun_Reload
	};
}

namespace Core::Music{
	enum class Id{
		MainTheme,
		Multiplayer,
	};
}

using TextureHolder = Core::ResourceManager<sf::Texture, Core::Textures::Id>;
using FontHolder = Core::ResourceManager<sf::Font, Core::Fonts::Id>;
using SoundBufferHolder = Core::ResourceManager<sf::SoundBuffer, Core::SoundBuffers::Id>;
using MusicHolder = Core::ResourceManager<sf::Music, Core::Music::Id>;

struct Resources{
	TextureHolder Textures;
	FontHolder Fonts;
	SoundBufferHolder SoundBuffers;
	MusicHolder Music;
	std::vector<std::unique_ptr<sf::Sound>> Sounds{};
};