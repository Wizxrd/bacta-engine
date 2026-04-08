#include "Core/Player.hpp"

#include <math.h>

namespace Core{

	Player::Player(StateContext& context, Soldier& soldier) : mContext(context), mSoldier(&soldier){
		mMovementKeybinds[sf::Keyboard::Scancode::W] = MovementAction::MoveUp;
		mMovementKeybinds[sf::Keyboard::Scancode::S] = MovementAction::MoveDown;
		mMovementKeybinds[sf::Keyboard::Scancode::A] = MovementAction::MoveLeft;
		mMovementKeybinds[sf::Keyboard::Scancode::D] = MovementAction::MoveRight;

		mAttackKeybinds[sf::Keyboard::Scancode::F] = AttackAction::Melee;

		mWeaponKeybinds[sf::Keyboard::Scancode::Num1] = WeaponAction::Rifle;
		mWeaponKeybinds[sf::Keyboard::Scancode::Num2] = WeaponAction::Shotgun;
		mWeaponKeybinds[sf::Keyboard::Scancode::Num3] = WeaponAction::Handgun;
		mWeaponKeybinds[sf::Keyboard::Scancode::V] = WeaponAction::Knife;
		mWeaponKeybinds[sf::Keyboard::Scancode::R] = WeaponAction::Reload;
	}

	void Player::HandleEvent(const sf::Event& event){
		if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()){
			if (mMovementKeybinds[keyPressed->scancode]){
			}
			if (mAttackKeybinds[keyPressed->scancode]){
			}
		}
	}

	bool Player::IsActionPressedOnce(sf::Keyboard::Scancode scancode){
		const bool isDown = sf::Keyboard::isKeyPressed(scancode);
		const bool wasDown = mPreviousKeyStates[scancode];
		mPreviousKeyStates[scancode] = isDown;
		return isDown && !wasDown;
	}

	bool Player::IsMousePressedOnce(sf::Mouse::Button button){
		const bool isDown = sf::Mouse::isButtonPressed(button);
		const bool wasDown = mPreviousMouseStates[button];
		mPreviousMouseStates[button] = isDown;
		return isDown && !wasDown;
	}

	void Player::HandleInput(float radians){
		bool wantsShoot = false;
		bool shotThisFrame = false;
		bool walking = false;
		bool running = false;
		bool strafeLeft = false;
		bool strafeRight = false;
		bool melee = false, reload = false;
		const bool mouseHeld = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		const bool mousePressedOnce = IsMousePressedOnce(sf::Mouse::Button::Left);
		for (const auto& [scancode, action] : mMovementKeybinds){
			if (!sf::Keyboard::isKeyPressed(scancode)){
				continue;
			}
			walking = true;
			const float radians = mSoldier->GetAimAngle().asRadians();
			running = sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::LShift);
			switch (action){
				case MoveUp:
					mSoldier->Accelerate({ std::cos(radians), std::sin(radians) });
					break;
				case MoveDown:
					mSoldier->Accelerate({ -std::cos(radians), -std::sin(radians) });
					break;
				case MoveLeft:
					strafeLeft = true;
					walking = false;
					running = false;
					mSoldier->Accelerate({ std::sin(radians), -std::cos(radians) });
					break;
				case MoveRight:
					strafeRight = true;
					walking = false;
					running = false;
					mSoldier->Accelerate({ -std::sin(radians), std::cos(radians) });
					break;
			}
		}

		for (const auto& [scancode, action] : mAttackKeybinds){
			if (!sf::Keyboard::isKeyPressed(scancode)){
				continue;
			}
			switch (action){
				case Melee:
					melee = true;
					break;
			}
		}

		for (const auto& [scancode, action] : mWeaponKeybinds){
			const bool pressedOnce = IsActionPressedOnce(scancode);
			switch (action){
				case Knife:
					if (pressedOnce){
						mSoldier->RequestWeapon(Core::Soldier::WeaponState::Knife);
					}
					break;
				case Handgun:
					if (pressedOnce){
						mSoldier->RequestWeapon(Core::Soldier::WeaponState::Handgun);
					}
					break;
				case Rifle:
					if (pressedOnce){
						mSoldier->RequestWeapon(Core::Soldier::WeaponState::Rifle);
					}
					break;
				case Shotgun:
					if (pressedOnce){
						mSoldier->RequestWeapon(Core::Soldier::WeaponState::Shotgun);
					}
					break;
				case Reload:
					if (pressedOnce){
						mSoldier->IsReloading = true;
						mSoldier->PlayReloadSound();
						reload = true;
					}
					break;
			}
		}

		if (mSoldier->mWeaponState == Core::Soldier::WeaponState::Rifle){
			if (mouseHeld){
				wantsShoot = true;
			}
		} else{
			if (mousePressedOnce){
				wantsShoot = true;
			}
		}

		if (wantsShoot && !mSoldier->IsReloading){
			shotThisFrame = mSoldier->TryShoot();
		}

		bool animateShoot = false;

		if (mSoldier->mWeaponState == Core::Soldier::WeaponState::Rifle){
			animateShoot = wantsShoot;
		} else{
			animateShoot = shotThisFrame;
		}

		mSoldier->AnimateFeet(walking, running, strafeLeft, strafeRight);
		mSoldier->AnimateBody(melee, walking, reload, animateShoot);
	}

	void Player::Update(sf::Time deltaTime){

	}
}