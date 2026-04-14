#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

class Camera{
	public:
		Camera(sf::RenderWindow& window, sf::View& view);
		void Update(sf::Vector2f position, sf::Time deltaTime);
		void Zoom(float amount);
	private:
		void updateLerp(sf::Vector2f position, sf::Time deltaTime);
		void updateZoom(sf::Time deltaTime);
	private:
		sf::RenderWindow& mWindow;
		sf::View& mView;
		float mZoomLevel = 1000.f;
		float mTargetZoom = 1000.f;
		float mCameraSmooting = 5.f;
		float mZoomSmoothing = 2.5f;
};
