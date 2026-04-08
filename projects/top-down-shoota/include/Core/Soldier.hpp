#pragma once

#include <Core/Animator.hpp>
#include <Core/Entity.hpp>
#include "Core/StateContext.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <map>

namespace Core{
	class Soldier : public Entity{
		public:
			enum class FeetState{
				Idle,
				Walk,
				StrafeLeft,
				StrafeRight,
				Run
			};

			enum class BodyState{
				Idle,
				Melee,
				Move,
				Reload,
				Shoot
			};

			enum class WeaponState{
				None,
				Knife,
				Handgun,
				Shotgun,
				Rifle
			};

			struct BodyAnimationState{
				Core::Textures::Id textureId;
				Animation* animation;
				sf::Vector2f origin = {-1.f, -1.f};
			};

		public:
			explicit Soldier(Core::StateContext& context);
			void Update(sf::Time deltaTime);
			void UpdateImGui(sf::Time deltaTime);
			void Render(sf::RenderTarget& target);
		public:
			const sf::Vector2f GetCenter() const;
			const sf::Vector2f GetPosition() const;
			const sf::Angle GetAimAngle() const { return mAimAngle; }
			void SetPosition(sf::Vector2f position);
			void SetAimAngle(sf::Angle angle);
			void Move(sf::Time deltaTime);
			void AnimateFeet(bool walking, bool running, bool strafeLeft, bool strafeRight);
			void AnimateBody(bool melee, bool move, bool reload, bool shoot);
			bool IsWalking() const { return mFeetState == FeetState::Walk; }
			bool IsRunning() const{ return mFeetState == FeetState::Run; }
			void RequestWeapon(WeaponState weapon);
			bool TryShoot();
			void PlayReloadSound();
			const BodyAnimationState& GetCurrentBodyAnimation() const;
		public:
			bool IsShooting = false;
			bool IsReloading = false;
			float mTimeSinceReload = 0.f;
			Animator mBodyAnimator;
			WeaponState mWeaponState = WeaponState::Knife;
			BodyState mBodyState = BodyState::Idle;
		private:
			Core::StateContext& mContext;
			sf::Angle mAimAngle;
			sf::Sprite mBody;
			sf::Sprite mFeet;
			//Animator mBodyAnimator;
			Animator mFeetAnimator;

			Animation mFeetIdleAnimation;
			Animation mFeetRunAnimation;
			Animation mFeetStrafeLeftAnimation;
			Animation mFeetStrafeRightAnimation;
			Animation mFeetWalkAnimation;

			Animation mFlashlightIdleAnimation;
			Animation mFlashlightMeleeAnimation;
			Animation mFlashlightMoveAnimation;

			Animation mHandgunIdleAnimation;
			Animation mHandgunMeleeAnimation;
			Animation mHandgunMoveAnimation;
			Animation mHandgunReloadAnimation;
			Animation mHandgunShootAnimation;

			Animation mKnifeIdleAnimation;
			Animation mKnifeMeleeAnimation;
			Animation mKnifeMoveAnimation;

			Animation mRifleIdleAnimation;
			Animation mRifleMeleeAnimation;
			Animation mRifleMoveAnimation;
			Animation mRifleReloadAnimation;
			Animation mRifleShootAnimation;

			Animation mShotgunIdleAnimation;
			Animation mShotgunMeleeAnimation;
			Animation mShotgunMoveAnimation;
			Animation mShotgunReloadAnimation;
			Animation mShotgunShootAnimation;

			FeetState mFeetState = FeetState::Idle;
			//BodyState mBodyState = BodyState::Idle;
			//WeaponState mWeaponState = WeaponState::Knife;
			WeaponState mPendingWeapon = WeaponState::None;

			std::map<BodyState, BodyAnimationState> mHandgunAnimations;
			std::map<BodyState, BodyAnimationState> mKnifeAnimations;
			std::map<BodyState, BodyAnimationState> mRifleAnimations;
			std::map<BodyState, BodyAnimationState> mShotgunAnimations;

			std::map<WeaponState, float> mFireRates;

			float mWalkSpeed = 300.f;
			float mTimeSinceLastShot = 0.f;

			float XOffset = 0.f;
			float YOffset = 0.f;
			float mReloadDuration = 1.f;
			bool mReloadComplete = false;

	};
}