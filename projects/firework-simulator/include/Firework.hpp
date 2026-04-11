#pragma once

#include "AerialShell.hpp"

#include <SFML/System/Vector2.hpp>
#include <SFML/Graphics.hpp>

#include <vector>

class Firework{
	public:
		Firework(const AerialShell& shell) : mAerialShell(shell){}
		void Launch(sf::Vector2f origin);
		void Detonate(sf::Vector2f origin);
		void Update(sf::Time deltaTime);
		void Render(sf::RenderTarget& target);
	public:
		bool IsCompleted();
		std::size_t GetMemoryBytes() const;
	private:
		float getExplosivePower(const ShellData::Composition& composition);
		float getMass(const ShellData::Composition& composition);
		void updateLiftCharge(sf::Time deltaTime);
		void updateBurstCharge(sf::Time deltaTime);
	private:
		AerialShell mAerialShell;
};