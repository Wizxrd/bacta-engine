#include "Core/SoldierAnimations.hpp"

#include <imgui.h>

#include <cmath>
#include <stdexcept>

namespace Core{

	namespace{
		constexpr float kOriginReferenceX(const Animation& rifleIdle){
			return rifleIdle.Frames[0].size.x / 2.f - 25.f;
		}
		constexpr float kOriginReferenceY(const Animation& rifleIdle){
			return rifleIdle.Frames[0].size.y / 2.f + 25.f;
		}
		constexpr float kOriginMeleeYOffset(const Animation& rifleIdle){
			return rifleIdle.Frames[0].size.y / 2.f + 100.f;
		}
	}

	SoldierAnimations::SoldierAnimations(Resources& resources) :
		mResources(resources),
		mBody(mResources.Textures.Get(Textures::Id::Player_Knife_Idle)),
		mFeet(mResources.Textures.Get(Textures::Id::Player_Feet_Idle)),

		mFeetIdleAnimation(mResources.Textures.Get(Textures::Id::Player_Feet_Idle), 1),
		mFeetRunAnimation(mResources.Textures.Get(Textures::Id::Player_Feet_Run), 20),
		mFeetStrafeLeftAnimation(mResources.Textures.Get(Textures::Id::Player_Feet_Strafe_Left), 20),
		mFeetStrafeRightAnimation(mResources.Textures.Get(Textures::Id::Player_Feet_Strafe_Right), 20),
		mFeetWalkAnimation(mResources.Textures.Get(Textures::Id::Player_Feet_Walk), 20),

		mFlashlightIdleAnimation(mResources.Textures.Get(Textures::Id::Player_Flashlight_Idle), 20),
		mFlashlightMeleeAnimation(mResources.Textures.Get(Textures::Id::Player_Flashlight_Melee), 15),
		mFlashlightMoveAnimation(mResources.Textures.Get(Textures::Id::Player_Flashlight_Move), 20),

		mHandgunIdleAnimation(mResources.Textures.Get(Textures::Id::Player_Handgun_Idle), 20),
		mHandgunMeleeAnimation(mResources.Textures.Get(Textures::Id::Player_Handgun_Melee), 15),
		mHandgunMoveAnimation(mResources.Textures.Get(Textures::Id::Player_Handgun_Move), 20),
		mHandgunReloadAnimation(mResources.Textures.Get(Textures::Id::Player_Handgun_Reload), 15),
		mHandgunShootAnimation(mResources.Textures.Get(Textures::Id::Player_Handgun_Shoot), 3),

		mKnifeIdleAnimation(mResources.Textures.Get(Textures::Id::Player_Knife_Idle), 20),
		mKnifeMeleeAnimation(mResources.Textures.Get(Textures::Id::Player_Knife_Melee), 15),
		mKnifeMoveAnimation(mResources.Textures.Get(Textures::Id::Player_Knife_Move), 20),

		mRifleIdleAnimation(mResources.Textures.Get(Textures::Id::Player_Rifle_Idle), 20),
		mRifleMeleeAnimation(mResources.Textures.Get(Textures::Id::Player_Rifle_Melee), 15),
		mRifleMoveAnimation(mResources.Textures.Get(Textures::Id::Player_Rifle_Move), 20),
		mRifleReloadAnimation(mResources.Textures.Get(Textures::Id::Player_Rifle_Reload), 20),
		mRifleShootAnimation(mResources.Textures.Get(Textures::Id::Player_Rifle_Shoot), 3),

		mShotgunIdleAnimation(mResources.Textures.Get(Textures::Id::Player_Shotgun_Idle), 20),
		mShotgunMeleeAnimation(mResources.Textures.Get(Textures::Id::Player_Shotgun_Melee), 15),
		mShotgunMoveAnimation(mResources.Textures.Get(Textures::Id::Player_Shotgun_Move), 20),
		mShotgunReloadAnimation(mResources.Textures.Get(Textures::Id::Player_Shotgun_Reload), 20),
		mShotgunShootAnimation(mResources.Textures.Get(Textures::Id::Player_Shotgun_Shoot), 3)
	{
		mFeet.setPosition({1000.f, 1000.f});
		mFeet.setOrigin({
			mFeetIdleAnimation.Frames[0].size.x / 2.f,
			mKnifeIdleAnimation.Frames[0].size.y / 2.f,
		});
		mBody.setPosition(mFeet.getPosition());
		mBody.setOrigin({
			mKnifeIdleAnimation.Frames[0].size.x / 2.f,
			mKnifeIdleAnimation.Frames[0].size.y / 2.f,
		});

		mFeet.setRotation(sf::degrees(-45.f));

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

		buildBodyMaps(mRifleIdleAnimation);
	}

	void SoldierAnimations::buildBodyMaps(const Animation& rifleIdle){
		const float ox = kOriginReferenceX(rifleIdle);
		const float oy = kOriginReferenceY(rifleIdle);
		const float my = kOriginMeleeYOffset(rifleIdle);

		mHandgunClips[BodyState::Idle] = {Textures::Id::Player_Handgun_Idle, &mHandgunIdleAnimation, {ox, oy}};
		mHandgunClips[BodyState::Melee] = {Textures::Id::Player_Handgun_Melee, &mHandgunMeleeAnimation, {ox, oy}};
		mHandgunClips[BodyState::Move] = {Textures::Id::Player_Handgun_Move, &mHandgunMoveAnimation, {ox, oy}};
		mHandgunClips[BodyState::Reload] = {Textures::Id::Player_Handgun_Reload, &mHandgunReloadAnimation, {ox, oy}};
		mHandgunClips[BodyState::Shoot] = {Textures::Id::Player_Handgun_Shoot, &mHandgunShootAnimation, {ox, oy}};

		mKnifeClips[BodyState::Idle] = {Textures::Id::Player_Knife_Idle, &mKnifeIdleAnimation, {ox, oy}};
		mKnifeClips[BodyState::Melee] = {Textures::Id::Player_Knife_Melee, &mKnifeMeleeAnimation, {ox, oy}};
		mKnifeClips[BodyState::Move] = {Textures::Id::Player_Knife_Move, &mKnifeMoveAnimation, {ox, oy}};

		mRifleClips[BodyState::Idle] = {Textures::Id::Player_Rifle_Idle, &mRifleIdleAnimation, {ox, oy}};
		mRifleClips[BodyState::Melee] = {Textures::Id::Player_Rifle_Melee, &mRifleMeleeAnimation, {ox, my}};
		mRifleClips[BodyState::Move] = {Textures::Id::Player_Rifle_Move, &mRifleMoveAnimation, {ox, oy}};
		mRifleClips[BodyState::Reload] = {Textures::Id::Player_Rifle_Reload, &mRifleReloadAnimation, {ox, oy}};
		mRifleClips[BodyState::Shoot] = {Textures::Id::Player_Rifle_Shoot, &mRifleShootAnimation, {ox, oy}};

		mShotgunClips[BodyState::Idle] = {Textures::Id::Player_Shotgun_Idle, &mShotgunIdleAnimation, {ox, oy}};
		mShotgunClips[BodyState::Melee] = {Textures::Id::Player_Shotgun_Melee, &mShotgunMeleeAnimation, {ox, my}};
		mShotgunClips[BodyState::Move] = {Textures::Id::Player_Shotgun_Move, &mShotgunMoveAnimation, {ox, oy}};
		mShotgunClips[BodyState::Reload] = {Textures::Id::Player_Shotgun_Reload, &mShotgunReloadAnimation, {ox, oy}};
		mShotgunClips[BodyState::Shoot] = {Textures::Id::Player_Shotgun_Shoot, &mShotgunShootAnimation, {ox, oy}};
	}

	void SoldierAnimations::Update(sf::Time deltaTime){
		mFeetAnimator.Update(deltaTime);
		mBodyAnimator.Update(deltaTime);

		mBody.setRotation(mAimAngle);
		if (mBodyState == BodyState::Idle && mWeaponState == WeaponState::Knife){
			mFeet.setRotation(mAimAngle - sf::degrees(45.f));
		} else{
			mFeet.setRotation(mAimAngle);
		}
	}

	void SoldierAnimations::UpdateImGui(sf::Time){
		ImGui::Begin("Soldier animations");
		ImGui::SliderFloat("Body X offset", &XOffset, -100.f, 100.f);
		ImGui::SliderFloat("Body Y offset", &YOffset, -100.f, 100.f);
		ImGui::End();
	}

	void SoldierAnimations::Render(sf::RenderTarget& target){
		mFeetAnimator.Render(target);
		mBodyAnimator.Render(target);
	}

	void SoldierAnimations::SetPosition(sf::Vector2f position){
		mBody.setPosition(position);
		mFeet.setPosition(position);
	}

	sf::Vector2f SoldierAnimations::GetPosition() const{
		return mBody.getPosition();
	}

	sf::Vector2f SoldierAnimations::GetCenter() const{
		return mBody.getGlobalBounds().getCenter();
	}

	void SoldierAnimations::SetAimAngle(sf::Angle angle){
		mAimAngle = angle;
	}

	void SoldierAnimations::RequestWeapon(WeaponState weapon){
		mPendingWeapon = weapon;
	}

	const SoldierAnimations::BodyClip& SoldierAnimations::resolveBodyClip() const{
		switch (mWeaponState){
			case WeaponState::Knife:
				return mKnifeClips.at(mBodyState);
			case WeaponState::Handgun:
				return mHandgunClips.at(mBodyState);
			case WeaponState::Shotgun:
				return mShotgunClips.at(mBodyState);
			case WeaponState::Rifle:
				return mRifleClips.at(mBodyState);
			case WeaponState::None:
				break;
		}
		return mKnifeClips.at(BodyState::Idle);
	}

	const SoldierAnimations::BodyClip& SoldierAnimations::GetCurrentBodyClip() const{
		return resolveBodyClip();
	}

	void SoldierAnimations::applyFeetClip(FeetState state, std::size_t carryFrameFromPrevious){
		switch (state){
			case FeetState::Idle:
				mFeet.setTexture(mResources.Textures.Get(Textures::Id::Player_Feet_Idle));
				mFeet.setOrigin({
					mFeetIdleAnimation.Frames[0].size.x / 2.f,
					mFeetIdleAnimation.Frames[0].size.y / 2.f,
				});
				mFeetAnimator.Play(mFeetIdleAnimation);
				break;
			case FeetState::Walk:
				mFeet.setTexture(mResources.Textures.Get(Textures::Id::Player_Feet_Walk));
				mFeet.setOrigin({
					mFeetWalkAnimation.Frames[0].size.x / 2.f,
					mFeetWalkAnimation.Frames[0].size.y / 2.f
				});
				mFeetAnimator.Play(mFeetWalkAnimation);
				mFeetAnimator.mFrame = carryFrameFromPrevious;
				break;
			case FeetState::Run:
				mFeet.setTexture(mResources.Textures.Get(Textures::Id::Player_Feet_Run));
				mFeet.setOrigin({
					mFeetRunAnimation.Frames[0].size.x / 2.f,
					mFeetRunAnimation.Frames[0].size.y / 2.f
				});
				mFeetAnimator.Play(mFeetRunAnimation);
				mFeetAnimator.mFrame = carryFrameFromPrevious;
				break;
			case FeetState::StrafeLeft:
				mFeet.setTexture(mResources.Textures.Get(Textures::Id::Player_Feet_Strafe_Left));
				mFeet.setOrigin({
					mFeetStrafeLeftAnimation.Frames[0].size.x / 2.f,
					mFeetStrafeLeftAnimation.Frames[0].size.y / 2.f
				});
				mFeetAnimator.Play(mFeetStrafeLeftAnimation);
				mFeetAnimator.mFrame = carryFrameFromPrevious;
				break;
			case FeetState::StrafeRight:
				mFeet.setTexture(mResources.Textures.Get(Textures::Id::Player_Feet_Strafe_Right));
				mFeet.setOrigin({
					mFeetStrafeRightAnimation.Frames[0].size.x / 2.f,
					mFeetStrafeRightAnimation.Frames[0].size.y / 2.f
				});
				mFeetAnimator.Play(mFeetStrafeRightAnimation);
				mFeetAnimator.mFrame = carryFrameFromPrevious;
				break;
		}
	}

	void SoldierAnimations::AnimateFeet(bool walking, bool running, bool strafeLeft, bool strafeRight){
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
		applyFeetClip(mFeetState, previousFrame);
	}

	void SoldierAnimations::AnimateBody(bool melee, bool move, bool reload, bool wantsShoot, bool shotThisFrame){
		bool animateShoot = false;
		if (mWeaponState == WeaponState::Rifle){
			animateShoot = wantsShoot;
		} else{
			animateShoot = shotThisFrame;
		}

		BodyState newState = BodyState::Idle;
		if (melee){
			newState = BodyState::Melee;
		} else if (animateShoot){
			newState = BodyState::Shoot;
		} else if (reload){
			newState = BodyState::Reload;
		} else if (move){
			newState = BodyState::Move;
		}

		const BodyClip& from = resolveBodyClip();
		const bool locked = from.animation && !from.animation->Loop && !mBodyAnimator.IsComplete();

		if (newState == mBodyState && mPendingWeapon == WeaponState::None){
			return;
		}
		if (mWeaponState == WeaponState::Knife && (wantsShoot || reload)){
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

		const BodyClip& to = resolveBodyClip();
		const sf::Vector2f pos = mFeet.getPosition();

		mBody.setTexture(mResources.Textures.Get(to.textureId));

		sf::Vector2f origin = to.origin;
		if (origin.x < 0.f || origin.y < 0.f){
			origin = {
				to.animation->Frames[0].size.x / 2.f + XOffset,
				to.animation->Frames[0].size.y / 2.f + YOffset
			};
		} else{
			origin.x += XOffset;
			origin.y += YOffset;
		}

		mBody.setOrigin(origin);
		mBody.setPosition(pos);
		mBodyAnimator.Play(*to.animation);
	}
}