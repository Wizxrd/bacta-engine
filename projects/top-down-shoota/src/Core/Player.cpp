#include "Core/Player.hpp"
#include "Core/Weapon.hpp"

#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>

#include <math.h>

namespace Core{

	struct Test{
		Test() = default;
	};

	Player::Player(Soldier& soldier, StateContext& context) : mSoldier(soldier), mContext(context){
		mKeybinds[sf::Keyboard::Scancode::W] = Keybind::MoveUp;
		mKeybinds[sf::Keyboard::Scancode::S] = Keybind::MoveDown;
		mKeybinds[sf::Keyboard::Scancode::A] = Keybind::MoveLeft;
		mKeybinds[sf::Keyboard::Scancode::D] = Keybind::MoveRight;
		mKeybinds[sf::Keyboard::Scancode::LShift] = Keybind::Sprint;

		mKeybinds[sf::Keyboard::Scancode::F] = Keybind::Melee;
		mKeybinds[sf::Keyboard::Scancode::V] = Keybind::Knife;
		mKeybinds[sf::Keyboard::Scancode::Num3] = Keybind::Handgun;
		mKeybinds[sf::Keyboard::Scancode::Num2] = Keybind::Shotgun;
		mKeybinds[sf::Keyboard::Scancode::Num1] = Keybind::Rifle;
		mKeybinds[sf::Keyboard::Scancode::R] = Keybind::Reload;
	}

	void Player::Update(){
		auto& input = mContext.mInputManager;

		mSoldier.SetAim(GetAim()); // radians
		mSoldier.SetVelocity({0.f, 0.f});
		const float aim = mSoldier.GetAim();

		bool walking = false;
		bool running = false;
		bool strafeLeft = false;
		bool strafeRight = false;
		bool melee = false;
		bool reload = false;
		bool animateShoot = false;

		// HELD: movement/sprint
		for (const auto& [scancode, action] : mKeybinds){
			if (!input.IsKeyHeld(scancode)){
				continue;
			}
			switch (action){
				case Keybind::MoveUp:
					walking = true;
					mSoldier.Accelerate({std::cos(aim), std::sin(aim)});
					break;
				case Keybind::MoveDown:
					walking = true;
					mSoldier.Accelerate({-std::cos(aim), -std::sin(aim)});
					break;
				case Keybind::MoveLeft:
					strafeLeft = true;
					mSoldier.Accelerate({std::sin(aim), -std::cos(aim)});
					break;
				case Keybind::MoveRight:
					strafeRight = true;
					mSoldier.Accelerate({-std::sin(aim), std::cos(aim)});
					break;
				case Keybind::Sprint:
					running = true;
					break;
				default:
					break;
			}
		}

		if (strafeLeft || strafeRight){
			walking = false;
			running = false;
		}

		mSoldier.SetRunning(running);

		// PRESSED: one-shot actions
		for (const auto& [scancode, action] : mKeybinds){
			if (!input.IsKeyPressed(scancode)){
				continue;
			}

			switch (action){
				case Keybind::Melee:
					melee = true;
					mSoldier.Melee();
					break;
				case Keybind::Knife:
					mSoldier.RequestWeapon(Weapon::Type::Knife);
					break;
				case Keybind::Handgun:
					mSoldier.RequestWeapon(Weapon::Type::Handgun);
					break;
				case Keybind::Shotgun:
					mSoldier.RequestWeapon(Weapon::Type::Shotgun);
					break;
				case Keybind::Rifle:
					mSoldier.RequestWeapon(Weapon::Type::Rifle);
					break;
				case Keybind::Reload:
					reload = true;
					mSoldier.ReloadWeapon();
					break;
				default:
					break;
			}
		}

		const bool wantsShoot = (mSoldier.GetWeapon() == Weapon::Type::Rifle)
			? input.IsMouseHeld(sf::Mouse::Button::Left)
			: input.IsMousePressed(sf::Mouse::Button::Left);

		bool shotThisFrame = false;
		if (wantsShoot){
			shotThisFrame = mSoldier.FireWeapon();
		}

		mSoldier.AnimateFeet(walking, running, strafeLeft, strafeRight);
		mSoldier.AnimateBody(melee, walking, reload, wantsShoot, shotThisFrame);
	}

	const float Player::GetAim(){
		const sf::Vector2i pixel = sf::Mouse::getPosition(mContext.mWindow);
		const sf::Vector2f mouseWorld = mContext.mWindow.mapPixelToCoords(pixel, mContext.mView);
		const sf::Vector2f pivot = mSoldier.GetCenter();
		const sf::Vector2f delta = mouseWorld - pivot;
		const float rad = std::atan2(delta.y, delta.x);
		return rad;
	}
}