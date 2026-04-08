#pragma once

#include "Core/Soldier.hpp"
#include <Core/StateContext.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Time.hpp>

#include <map>
#include <functional>

namespace Core{
	class Player{
		public:
			enum MovementAction{
				MoveLeft,
				MoveRight,
				MoveUp,
				MoveDown,
			};

			enum AttackAction{
				Melee,
			};

			enum WeaponAction{
				Knife,
				Handgun,
				Rifle,
				Shotgun,
				Reload,
				Shoot
			};

		public:
			Player(StateContext& context, Soldier& entity);
			void HandleEvent(const sf::Event& event);
			void HandleInput(float radians);
			void Update(sf::Time deltaTime);
			bool IsActionPressedOnce(sf::Keyboard::Scancode scancode);
			bool IsMousePressedOnce(sf::Mouse::Button button);
		private:
			StateContext& mContext;
			Soldier* mSoldier;
			std::map<sf::Keyboard::Scancode, MovementAction> mMovementKeybinds;
			std::map<sf::Keyboard::Scancode, AttackAction> mAttackKeybinds;
			std::map<sf::Keyboard::Scancode, WeaponAction> mWeaponKeybinds;
			std::unordered_map<sf::Keyboard::Scancode, bool> mPreviousKeyStates;
			std::unordered_map<sf::Mouse::Button, bool> mPreviousMouseStates;
	};
}