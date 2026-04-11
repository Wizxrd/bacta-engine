#include "Core/Camera.hpp"

#include <math.h>

namespace Core{

	Camera::Camera(StateContext& context) : mContext(context){}

	void Camera::Update(sf::Vector2f position, sf::Time deltaTime){
		updateLerp(position, deltaTime);
		updateZoom(deltaTime);
	}

	void Camera::updateLerp(sf::Vector2f position, sf::Time deltaTime){
		const float interpolation = 1.f - std::exp(-mCameraSmooting * deltaTime.asSeconds());
		sf::Vector2f center = mContext.mView.getCenter();
		mContext.mView.setCenter(center + (position - center) * interpolation);
	}

	void Camera::updateZoom(sf::Time deltaTime){
		const float aspect = static_cast<float>(mContext.mWindow.getSize().x) /
							 static_cast<float>(mContext.mWindow.getSize().y);
		const float interpolation = 1.f - std::exp(-mCameraSmooting * deltaTime.asSeconds());
		mZoomLevel += (mTargetZoom - mZoomLevel) * interpolation;
		mContext.mView.setSize({mZoomLevel * aspect, mZoomLevel});
		mContext.mView.setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));
	}
}