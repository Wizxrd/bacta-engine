#include "camera.hpp"
#include <math.h>

Camera::Camera(sf::RenderWindow& window, sf::View& view) : mWindow(window), mView(view){}

void Camera::Update(sf::Vector2f position, sf::Time deltaTime){
	updateLerp(position, deltaTime);
	updateZoom(deltaTime);
}

void Camera::Zoom(float ammount){
	mTargetZoom += ammount;
}

void Camera::updateLerp(sf::Vector2f position, sf::Time deltaTime){
	const float interpolation = 1.f - std::exp(-mCameraSmooting * deltaTime.asSeconds());
	sf::Vector2f center = mView.getCenter();
	mView.setCenter(center + (position - center) * interpolation);
}

void Camera::updateZoom(sf::Time deltaTime){
	const float aspect = static_cast<float>(mWindow.getSize().x) /
							static_cast<float>(mWindow.getSize().y);
	const float interpolation = 1.f - std::exp(-mCameraSmooting * deltaTime.asSeconds());
	mZoomLevel += (mTargetZoom - mZoomLevel) * interpolation;
	mView.setSize({mZoomLevel * aspect, mZoomLevel});
	mView.setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));
}