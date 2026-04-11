#pragma once

#include "Core/Animation.hpp"
#include "Core/Animator.hpp"
#include "Core/Resources.hpp"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Angle.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <map>

namespace Core{
	class SoldierAnimations{
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

			struct BodyClip{
				Textures::Id textureId = Textures::Id::Player_Knife_Idle;
				Animation* animation = nullptr;
				sf::Vector2f origin = {-1.f, -1.f};
			};

		public:
			explicit SoldierAnimations(Resources& resources);

			void Update(sf::Time deltaTime);
			void Render(sf::RenderTarget& target);

			void UpdateImGui(sf::Time deltaTime);

			void SetPosition(sf::Vector2f position);
			sf::Vector2f GetPosition() const;
			sf::Vector2f GetCenter() const;

			void SetAimAngle(sf::Angle angle);
			sf::Angle GetAimAngle() const { return mAimAngle; }

			void AnimateFeet(bool walking, bool running, bool strafeLeft, bool strafeRight);
			void AnimateBody(bool melee, bool move, bool reload, bool wantsShoot, bool shoot);

			void RequestWeapon(WeaponState weapon);

			WeaponState GetWeaponState() const { return mWeaponState; }
			BodyState GetBodyState() const { return mBodyState; }
			FeetState GetFeetState() const { return mFeetState; }

			const BodyClip& GetCurrentBodyClip() const;

			float XOffset = 0.f;
			float YOffset = 0.f;

		private:
			void buildBodyMaps(const Animation& rifleIdleFirstFrameSource);
			void applyFeetClip(FeetState state, std::size_t carryFrameFromPrevious);

			const BodyClip& resolveBodyClip() const;

			Resources& mResources;

			sf::Sprite mBody;
			sf::Sprite mFeet;

			Animator mBodyAnimator;
			Animator mFeetAnimator;

			sf::Angle mAimAngle = sf::degrees(0.f);

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

			WeaponState mWeaponState = WeaponState::Knife;
			WeaponState mPendingWeapon = WeaponState::None;

			BodyState mBodyState = BodyState::Idle;

			std::map<BodyState, BodyClip> mHandgunClips;
			std::map<BodyState, BodyClip> mKnifeClips;
			std::map<BodyState, BodyClip> mRifleClips;
			std::map<BodyState, BodyClip> mShotgunClips;
	};
}