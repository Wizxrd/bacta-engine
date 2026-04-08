#include "Core/Entity.hpp"

namespace Core{
	void Entity::SetVelocity(sf::Vector2f velocity){
		mVelocity = velocity;
	}

	sf::Vector2f Entity::GetVelocity() const{
		return mVelocity;
	}

	void Entity::Accelerate(sf::Vector2f velocity){
		mVelocity += velocity;
	}
}