#pragma once

#include "Core/Soldier.hpp"
#include "Core/StateContext.hpp"

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>

#include <map>

namespace Core{
	class Player{
		public:
			enum Keybind{
				MoveUp,
				MoveDown,
				MoveLeft,
				MoveRight,
				Sprint,

				Melee,

				Knife,
				Handgun,
				Shotgun,
				Rifle,

				Reload
			};
		public:
			Player(Soldier& soldier, StateContext& context);
			void Update();
			const float GetAim();
		private:
			Soldier& mSoldier;
			StateContext& mContext;
			std::map<sf::Keyboard::Scancode, Keybind> mKeybinds{};
	};
}