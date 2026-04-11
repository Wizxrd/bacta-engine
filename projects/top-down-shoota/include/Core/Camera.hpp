#pragma once

#include "Core/StateContext.hpp"

#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

namespace Core{
	class Camera{
		public:
			Camera(StateContext& context);
			void Update(sf::Vector2f position, sf::Time deltaTime);
		private:
			void updateLerp(sf::Vector2f position, sf::Time deltaTime);
			void updateZoom(sf::Time deltaTime);
		private:
			StateContext& mContext;
			float mZoomLevel = 1000.f;
			float mTargetZoom = 1000.f;
			float mCameraSmooting = 5.f;
			float mZoomSmoothing = 2.5f;
	};
}
