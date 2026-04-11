#include "Core/Weapon.hpp"

namespace Core{
	Weapon::Weapon() : mCurrent(Weapon::Type::Handgun), mCanFire(true), mCanReload(true){
		mTimings[Weapon::Type::Handgun] = {0.25f, 1.f};
		mTimings[Weapon::Type::Shotgun] = {1.f, 1.f};
		mTimings[Weapon::Type::Rifle] = {0.135f, 1.f};
	}

	void Weapon::Update(sf::Time deltaTime){
		mTimeSinceFiring += deltaTime.asSeconds();
		mTimeSinceReload += deltaTime.asSeconds();
		if (mTimeSinceFiring >= mTimings[mCurrent].Fire){
			mCanFire = true;
		}
		if (mTimeSinceReload >= mTimings[mCurrent].Reload){
			mCanReload = true;
		}
	}

	bool Weapon::TryFire(){
		if (!mCanFire){return false;}
		mTimeSinceFiring = 0.f;
		mCanFire = false;
		return true;
	}

	bool Weapon::TryReload(){
		if (!mCanReload){return false;}
		mTimeSinceReload = 0.f;
		mCanReload = false;
		return true;
	}

	void Weapon::Swap(Weapon::Type weapon){
		mCurrent = weapon;
	}
}