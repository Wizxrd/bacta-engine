#include "Core/Soldier.hpp"

#include <math.h>

namespace Core{
	Soldier::Soldier(StateContext& context) : mContext(context), mAnimations(context.mResources){
	}

	void Soldier::Update(sf::Time deltaTime){
		move(deltaTime);
		mAnimations.SetPosition(mPosition);
		mAnimations.SetAimAngle(sf::radians(mAimAngle));
		mAnimations.Update(deltaTime);
		mWeapon.Update(deltaTime);
	}

	void Soldier::Render(sf::RenderTarget& target){
		mAnimations.Render(target);
	}

	const sf::Vector2f Soldier::GetPosition() const{
		return mPosition;
	}
	const sf::Vector2f Soldier::GetCenter() const{
		return mAnimations.GetCenter();
	}

	float Soldier::GetAim(){
		return mAimAngle;
	}
	void Soldier::SetAim(float radians){
		mAimAngle = radians;
	}

	void Soldier::SetRunning(bool running){
		mRunning = running;
	}

	Weapon::Type Soldier::GetWeapon() const{
		return mWeapon.GetCurrent();
	}

	bool Soldier::FireWeapon(){
		if (mKnifeEquipped){
			return true;
		}
		if (!mWeapon.TryFire()){
			return false;
		}
		playGunSound();
		return true;
	}

	bool Soldier::ReloadWeapon(){
		if (mKnifeEquipped){
			return false;
		}
		if (!mWeapon.TryReload()){
			return false;
		}
		playReloadSound();
		return true;
	}

	void Soldier::RequestWeapon(Weapon::Type weapon){
		mKnifeEquipped = weapon == Weapon::Type::Knife ? true : false;
		mWeapon.Swap(weapon);
		SoldierAnimations::WeaponState animationWeapon = SoldierAnimations::WeaponState::None;
		switch (weapon){
			case Weapon::Type::Knife:
				animationWeapon = SoldierAnimations::WeaponState::Knife;
				break;
			case Weapon::Type::Handgun:
				animationWeapon = SoldierAnimations::WeaponState::Handgun;
				break;
			case Weapon::Type::Shotgun:
				animationWeapon = SoldierAnimations::WeaponState::Shotgun;
				break;
			case Weapon::Type::Rifle:
				animationWeapon = SoldierAnimations::WeaponState::Rifle;
				break;
		}
		mAnimations.RequestWeapon(animationWeapon);
	}

	void Soldier::AnimateFeet(bool walking, bool running, bool strafeLeft, bool strafeRight){
		mAnimations.AnimateFeet(walking, running, strafeLeft, strafeRight);
	}

	void Soldier::AnimateBody(bool melee, bool move, bool reload, bool wantsShoot, bool shotThisFrame){
		mAnimations.AnimateBody(melee, move, reload, wantsShoot, shotThisFrame);
	}
	//////////////////////////////////////////////////////////////////////

	void Soldier::move(sf::Time deltaTime){
		sf::Vector2f velocity = GetVelocity();
		if (velocity.x != 0.f || velocity.y != 0.f){
			const float len = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
			velocity /= len;
		}
		const float speed = (mRunning && !mKnifeEquipped) ? (mWalkSpeed * 2.f) : mWalkSpeed;
		mPosition += velocity * speed * deltaTime.asSeconds();
	}

	void Soldier::playGunSound(){
		Core::SoundBuffers::Id id = Core::SoundBuffers::Id::Handgun_Fire;
		switch (mWeapon.GetCurrent()){
			case Weapon::Type::Handgun:
				id = Core::SoundBuffers::Id::Handgun_Fire;
				break;
			case Weapon::Type::Rifle:
				id = Core::SoundBuffers::Id::Rifle_Fire;
				break;
			case Weapon::Type::Shotgun:
				id = Core::SoundBuffers::Id::Shotgun_Fire;
				break;
			default:
				break;
		}
		mContext.mSoundManager.PlayOnce(mContext.mResources.SoundBuffers.Get(id));
	}

	void Soldier::playReloadSound(){
		Core::SoundBuffers::Id id = Core::SoundBuffers::Id::Handgun_Reload;
		switch (mWeapon.GetCurrent()){
			case Weapon::Type::Handgun:
				id = Core::SoundBuffers::Id::Handgun_Reload;
				break;
			case Weapon::Type::Rifle:
				id = Core::SoundBuffers::Id::Rifle_Reload;
				break;
			case Weapon::Type::Shotgun:
				id = Core::SoundBuffers::Id::Shotgun_Reload;
				break;
			default:
				break;
		}
		mContext.mSoundManager.PlayOnce(mContext.mResources.SoundBuffers.Get(id));
	}
}