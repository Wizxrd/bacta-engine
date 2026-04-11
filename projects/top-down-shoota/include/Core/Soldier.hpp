#pragma once

#include "Core/Entity.hpp"
#include "Core/Weapon.hpp"
#include "Core/StateContext.hpp"
#include "Core/SoldierAnimations.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <SFML/Graphics/RectangleShape.hpp>

namespace Core{
	class Soldier : public Entity{
		public:
			Soldier(StateContext& context);
			void Update(sf::Time deltaTime);
			void Render(sf::RenderTarget& target);
			//
			const sf::Vector2f GetPosition() const;
			const sf::Vector2f GetCenter() const;
			//
			float GetAim();
			void SetAim(float radians);

			void SetRunning(bool running);

			Weapon::Type GetWeapon() const;

			bool FireWeapon();
			bool ReloadWeapon();
			void RequestWeapon(Weapon::Type weapon);

			void AnimateFeet(bool walking, bool running, bool strafeLeft, bool strafeRight);
			void AnimateBody(bool melee, bool move, bool reload, bool wantsShoot, bool shoot);
		private:
			void move(sf::Time deltaTime);
			void playGunSound();
			void playReloadSound();
		private:
			StateContext& mContext;
			Weapon mWeapon;
			SoldierAnimations mAnimations;

			sf::Vector2f mPosition = {1000.f, 1000.f};
			float mAimAngle = 0.f;

			bool mRunning = false;
			bool mKnifeEquipped = false;
			bool mIsReloading = false;
			float mWalkSpeed = 250.f;
	};
}