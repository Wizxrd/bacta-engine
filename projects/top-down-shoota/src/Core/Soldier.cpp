#include "Core/Soldier.hpp"
#include "Core/Resources.hpp"

#include <imgui.h>

#include <math.h>

/*
bugs:
rifle and shotgun reload animation
rifle and shotgun melee animation
*/

namespace Core{

	Soldier::Soldier(Core::StateContext& context) :
	mContext(context),
	mBody(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Knife_Idle)),
	mFeet(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Feet_Idle)),

	mFeetIdleAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Feet_Idle), 1),
	mFeetRunAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Feet_Run), 20),
	mFeetStrafeLeftAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Feet_Strafe_Left), 20),
	mFeetStrafeRightAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Feet_Strafe_Right), 20),
	mFeetWalkAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Feet_Walk), 20),

	mFlashlightIdleAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Flashlight_Idle), 20),
	mFlashlightMeleeAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Flashlight_Melee), 15),
	mFlashlightMoveAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Flashlight_Move), 20),

	mHandgunIdleAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Handgun_Idle), 20),
	mHandgunMeleeAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Handgun_Melee), 15),
	mHandgunMoveAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Handgun_Move), 20),
	mHandgunReloadAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Handgun_Reload), 15),
	mHandgunShootAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Handgun_Shoot), 3),

	mKnifeIdleAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Knife_Idle), 20),
	mKnifeMeleeAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Knife_Melee), 15),
	mKnifeMoveAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Knife_Move), 20),

	mRifleIdleAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Rifle_Idle), 20),
	mRifleMeleeAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Rifle_Melee), 15),
	mRifleMoveAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Rifle_Move), 20),
	mRifleReloadAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Rifle_Reload), 20),
	mRifleShootAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Rifle_Shoot), 3),

	mShotgunIdleAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Shotgun_Idle), 20),
	mShotgunMeleeAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Shotgun_Melee), 15),
	mShotgunMoveAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Shotgun_Move), 20),
	mShotgunReloadAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Shotgun_Reload), 20),
	mShotgunShootAnimation(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Shotgun_Shoot), 3)
	{
		mFeet.setPosition({1000.f,1000.f});
		mFeet.setOrigin({
			mFeetIdleAnimation.Frames[0].size.x/2.f,
			mKnifeIdleAnimation.Frames[0].size.y/2.f,
		});
		mBody.setPosition(mFeet.getPosition());
		mBody.setOrigin({
			mKnifeIdleAnimation.Frames[0].size.x/2.f,
			mKnifeIdleAnimation.Frames[0].size.y/2.f,
		});

		mFeet.setRotation(sf::degrees(-45));

		mFeetIdleAnimation.Loop = true;
		mFeetRunAnimation.Loop = true;
		mFeetStrafeLeftAnimation.Loop = true;
		mFeetStrafeRightAnimation.Loop = true;
		mFeetWalkAnimation.Loop = true;

		mFlashlightIdleAnimation.Loop = true;
		mFlashlightMeleeAnimation.Loop = false;
		mFlashlightMoveAnimation.Loop = true;

		mHandgunIdleAnimation.Loop = true;
		mHandgunMeleeAnimation.Loop = false;
		mHandgunMoveAnimation.Loop = true;
		mHandgunReloadAnimation.Loop = false;
		mHandgunShootAnimation.Loop = false;

		mKnifeIdleAnimation.Loop = true;
		mKnifeMeleeAnimation.Loop = false;
		mKnifeMoveAnimation.Loop = true;

		mRifleIdleAnimation.Loop = true;
		mRifleMeleeAnimation.Loop = false;
		mRifleMoveAnimation.Loop = true;
		mRifleReloadAnimation.Loop = false;
		mRifleShootAnimation.Loop = true;

		mShotgunIdleAnimation.Loop = true;
		mShotgunMeleeAnimation.Loop = false;
		mShotgunMoveAnimation.Loop = true;
		mShotgunReloadAnimation.Loop = false;
		mShotgunShootAnimation.Loop = false;

		mBodyAnimator.SetTarget(mBody);
		mFeetAnimator.SetTarget(mFeet);
		mBodyAnimator.Play(mKnifeIdleAnimation);
		mFeetAnimator.Play(mFeetIdleAnimation);

		mHandgunAnimations[BodyState::Idle] = {
			Core::Textures::Id::Player_Handgun_Idle,
			&mHandgunIdleAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mHandgunAnimations[BodyState::Melee] = {
			Core::Textures::Id::Player_Handgun_Melee,
			&mHandgunMeleeAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mHandgunAnimations[BodyState::Move] = {
			Core::Textures::Id::Player_Handgun_Move,
			&mHandgunMoveAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mHandgunAnimations[BodyState::Reload] = {
			Core::Textures::Id::Player_Handgun_Reload,
			&mHandgunReloadAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mHandgunAnimations[BodyState::Shoot] = {
			Core::Textures::Id::Player_Handgun_Shoot,
			&mHandgunShootAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};

		mKnifeAnimations[BodyState::Idle] = {
			Core::Textures::Id::Player_Knife_Idle,
			&mKnifeIdleAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mKnifeAnimations[BodyState::Melee] = {
			Core::Textures::Id::Player_Knife_Melee,
			&mKnifeMeleeAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mKnifeAnimations[BodyState::Move] = {
			Core::Textures::Id::Player_Knife_Move,
			&mKnifeMoveAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};

		mRifleAnimations[BodyState::Idle] = {
			Core::Textures::Id::Player_Rifle_Idle,
			&mRifleIdleAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mRifleAnimations[BodyState::Melee] = {
			Core::Textures::Id::Player_Rifle_Melee,
			&mRifleMeleeAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 100.f
			}
		};
		mRifleAnimations[BodyState::Move] = {
			Core::Textures::Id::Player_Rifle_Move,
			&mRifleMoveAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mRifleAnimations[BodyState::Reload] = {
			Core::Textures::Id::Player_Rifle_Reload,
			&mRifleReloadAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mRifleAnimations[BodyState::Shoot] = {
			Core::Textures::Id::Player_Rifle_Shoot,
			&mRifleShootAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};

		mShotgunAnimations[BodyState::Idle] = {
			Core::Textures::Id::Player_Shotgun_Idle,
			&mShotgunIdleAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mShotgunAnimations[BodyState::Melee] = {
			Core::Textures::Id::Player_Shotgun_Melee,
			&mShotgunMeleeAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 100.f
			}
		};
		mShotgunAnimations[BodyState::Move] = {
			Core::Textures::Id::Player_Shotgun_Move,
			&mShotgunMoveAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mShotgunAnimations[BodyState::Reload] = {
			Core::Textures::Id::Player_Shotgun_Reload,
			&mShotgunReloadAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};
		mShotgunAnimations[BodyState::Shoot] = {
			Core::Textures::Id::Player_Shotgun_Shoot,
			&mShotgunShootAnimation,
			{
				mRifleIdleAnimation.Frames[0].size.x / 2.f - 25.f,
				mRifleIdleAnimation.Frames[0].size.y / 2.f + 25.f
			}
		};

		mFireRates[WeaponState::Handgun] = 0.25;
		mFireRates[WeaponState::Rifle] = 0.135f;
		mFireRates[WeaponState::Shotgun] = 1.f;
	}

	// if we melee, since the sprite becomes bigger the origin needs to get set to where the center of the feet are

	void Soldier::Update(sf::Time deltaTime){
		mTimeSinceLastShot += deltaTime.asSeconds();
		if (mTimeSinceReload > mReloadDuration){
			IsReloading = false;
		}
		mTimeSinceReload += deltaTime.asSeconds();
		mFeetAnimator.Update(deltaTime);
		mBodyAnimator.Update(deltaTime);
		mBody.setRotation(mAimAngle);
		if (mBodyState == BodyState::Idle && mWeaponState == WeaponState::Knife){
			mFeet.setRotation(mAimAngle - sf::degrees(45.f));
		} else{
			mFeet.setRotation(mAimAngle);
		}
	}

	void Soldier::UpdateImGui(sf::Time deltaTime){
		ImGui::Begin("Soldier");
		ImGui::SliderFloat("Handgun", &mFireRates[WeaponState::Handgun], 0.01, 0.5f);
		ImGui::SliderFloat("Rifle", &mFireRates[WeaponState::Rifle], 0.01, 0.5f);
		ImGui::SliderFloat("Shotgun", &mFireRates[WeaponState::Shotgun], 0.01, 1.5f);
		ImGui::SliderFloat("XOffset", &XOffset, -100.f, 100.f);
		ImGui::SliderFloat("YOffset", &YOffset, -100.f, 100.f);
		ImGui::End();
	}

	void Soldier::Render(sf::RenderTarget& target){
		mFeetAnimator.Render(target);
		mBodyAnimator.Render(target);
	}

	const sf::Vector2f Soldier::GetCenter() const{
		return mBody.getGlobalBounds().getCenter();
	}

	const sf::Vector2f Soldier::GetPosition() const{
		return mBody.getPosition();
	}

	void Soldier::SetPosition(sf::Vector2f position){
		mBody.setPosition(position);
		mFeet.setPosition(position);
	}

	void Soldier::SetAimAngle(sf::Angle angle){
		mAimAngle = angle;
	}

	void Soldier::Move(sf::Time deltaTime){
		sf::Vector2f velocity = GetVelocity();
		auto pos = GetPosition();
		const float speed = mFeetState == FeetState::Run ? mWalkSpeed*2.f : mWalkSpeed;
		if (velocity.x != 0.f || velocity.y != 0.f){
			const float len = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
			velocity /= len;
		}
		pos += velocity * speed * deltaTime.asSeconds();
		SetPosition(pos);
	}

	void Soldier::AnimateFeet(bool walking, bool running, bool strafeLeft, bool strafeRight){
		FeetState newState = FeetState::Idle;
		if (running){
			newState = FeetState::Run;
		} else if (walking){
			newState = FeetState::Walk;
		} else if (strafeLeft){
			newState = FeetState::StrafeLeft;
		} else if (strafeRight){
			newState = FeetState::StrafeRight;
		}
		if (newState == mFeetState){
			return;
		}
		const std::size_t previousFrame = mFeetAnimator.mFrame;
		mFeetState = newState;
		switch (mFeetState){
			case FeetState::Idle:
				mFeet.setTexture(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Feet_Idle));
				mFeet.setOrigin({
					mFeetIdleAnimation.Frames[0].size.x / 2.f,
					mFeetIdleAnimation.Frames[0].size.y / 2.f,
				});
				mFeetAnimator.Play(mFeetIdleAnimation);
				break;
			case FeetState::Walk:
				mFeet.setTexture(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Feet_Walk));
				mFeet.setOrigin({
					mFeetWalkAnimation.Frames[0].size.x / 2.f,
					mFeetWalkAnimation.Frames[0].size.y / 2.f
				});
				mFeetAnimator.Play(mFeetWalkAnimation);
				mFeetAnimator.mFrame = previousFrame;
				break;
			case FeetState::Run:
				mFeet.setTexture(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Feet_Run));
				mFeet.setOrigin({
					mFeetRunAnimation.Frames[0].size.x / 2.f,
					mFeetRunAnimation.Frames[0].size.y / 2.f
				});
				mFeetAnimator.Play(mFeetRunAnimation);
				mFeetAnimator.mFrame = previousFrame;
				break;
			case FeetState::StrafeLeft:
				mFeet.setTexture(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Feet_Strafe_Left));
				mFeet.setOrigin({
					mFeetStrafeLeftAnimation.Frames[0].size.x / 2.f,
					mFeetStrafeLeftAnimation.Frames[0].size.y / 2.f
				});
				mFeetAnimator.Play(mFeetStrafeLeftAnimation);
				mFeetAnimator.mFrame = previousFrame;
				break;
			case FeetState::StrafeRight:
				mFeet.setTexture(mContext.mResources.Textures.Get(Core::Textures::Id::Player_Feet_Strafe_Right));
				mFeet.setOrigin({
					mFeetStrafeRightAnimation.Frames[0].size.x / 2.f,
					mFeetStrafeRightAnimation.Frames[0].size.y / 2.f
				});
				mFeetAnimator.Play(mFeetStrafeRightAnimation);
				mFeetAnimator.mFrame = previousFrame;
				break;
		}
	}

	void Soldier::AnimateBody(bool melee, bool move, bool reload, bool shoot){
		BodyState newState = BodyState::Idle;
		if (melee){
			newState = BodyState::Melee;
		} else if (shoot){
			newState = BodyState::Shoot;
		} else if (reload){
			newState = BodyState::Reload;
		} else if (move){
			newState = BodyState::Move;
		}
		const BodyAnimationState& from = GetCurrentBodyAnimation();
		const bool locked = from.animation && !from.animation->Loop && !mBodyAnimator.IsComplete();
		if (newState == mBodyState && mPendingWeapon == WeaponState::None){
			return;
		}
		if (mWeaponState == WeaponState::Knife && (shoot || reload)){
			return;
		}
		if (locked){
			return;
		}
		if (mPendingWeapon != WeaponState::None){
			mWeaponState = mPendingWeapon;
			mPendingWeapon = WeaponState::None;
		}
		mBodyState = newState;
		const BodyAnimationState& to = GetCurrentBodyAnimation();
		const sf::Vector2f pos = mFeet.getPosition();
		mBody.setTexture(mContext.mResources.Textures.Get(to.textureId));
		sf::Vector2f origin = to.origin;
		if (origin.x < 0.f || origin.y < 0.f){
			origin = {
				to.animation->Frames[0].size.x / 2.f + XOffset,
				to.animation->Frames[0].size.y / 2.f + YOffset
			};
		}
		else{
			origin.x += XOffset;
			origin.y += YOffset;
		}
		mBody.setOrigin(origin);
		mBody.setPosition(pos);
		mBodyAnimator.Play(*to.animation);
	}

	const Soldier::BodyAnimationState& Soldier::GetCurrentBodyAnimation() const{
		switch (mWeaponState){
			case WeaponState::Knife:
				return mKnifeAnimations.at(mBodyState);
			case WeaponState::Handgun:
				return mHandgunAnimations.at(mBodyState);
			case WeaponState::Shotgun:
				return mShotgunAnimations.at(mBodyState);
			case WeaponState::Rifle:
				return mRifleAnimations.at(mBodyState);
		}
		return mKnifeAnimations.at(BodyState::Idle);
	}

	void Soldier::RequestWeapon(WeaponState weapon){
		mPendingWeapon = weapon;
	}

	bool Soldier::TryShoot(){
		if (mWeaponState == WeaponState::Knife){
			return false;
		}
		if (mTimeSinceLastShot < mFireRates[mWeaponState]){
			return false;
		}
		mTimeSinceLastShot = 0.f;
		Core::SoundBuffers::Id buffer = Core::SoundBuffers::Id::Handgun_Fire;
		switch (mWeaponState){
			case WeaponState::Handgun:
				buffer = Core::SoundBuffers::Id::Handgun_Fire;
				break;
			case WeaponState::Rifle:
				buffer = Core::SoundBuffers::Id::Rifle_Fire;
				break;
			case WeaponState::Shotgun:
				buffer = Core::SoundBuffers::Id::Shotgun_Fire;
				break;
			default:
				break;
		}
		auto sound = std::make_unique<sf::Sound>(mContext.mResources.SoundBuffers.Get(buffer));
		mContext.mResources.Sounds.push_back(std::move(sound));
		mContext.mResources.Sounds.back()->play();
		return true;
	}

	void Soldier::PlayReloadSound(){
		if (mTimeSinceReload < mReloadDuration){ return; }
		mTimeSinceReload = 0.f;
		Core::SoundBuffers::Id buffer = Core::SoundBuffers::Id::Handgun_Reload;
		switch (mWeaponState){
			case WeaponState::Handgun:
				buffer = Core::SoundBuffers::Id::Handgun_Reload;
				break;
			case WeaponState::Rifle:
				buffer = Core::SoundBuffers::Id::Rifle_Reload;
				break;
			case WeaponState::Shotgun:
				buffer = Core::SoundBuffers::Id::Shotgun_Reload;
				break;
			default:
				break;
		}
		auto sound = std::make_unique<sf::Sound>(mContext.mResources.SoundBuffers.Get(buffer));
		mContext.mResources.Sounds.push_back(std::move(sound));
		mContext.mResources.Sounds.back()->play();
	}
}