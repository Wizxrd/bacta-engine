#pragma once

#include <SFML/System/Time.hpp>

#include <array>

namespace Core{
	class Weapon{
		public:
			enum Type{
				Knife,
				Handgun,
				Shotgun,
				Rifle,
				Count
			};
			struct Timing{
				float Fire = 1.f;
				float Reload = 1.f;
			};

		public:
			Weapon();
			void Update(sf::Time deltaTime);
			Weapon::Type GetCurrent() const { return mCurrent; };
			float GetReloadDuration() const { return mTimings[mCurrent].Reload;};
			bool TryFire();
			bool TryReload();
			void Swap(Weapon::Type weapon);
		private:
			Weapon::Type mCurrent;
			float mTimeSinceFiring = 0.f;
			float mTimeSinceReload = 0.f;
			std::array<Timing, Weapon::Type::Count> mTimings{};
			bool mCanFire = false;
			bool mCanReload = false;
	};
}