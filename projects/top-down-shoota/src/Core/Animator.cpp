#include "Core/Animator.hpp"

namespace Core{

	void Animator::Play(const Animation& animation){
		mAnimation = nullptr;
		mAnimation = &animation;
		mFrame = 0;
		mElapsed = 0.f;
		mComplete = false;
	}

	void Animator::Update(sf::Time deltaTime){
		if (!mAnimation || mAnimation->Frames.empty() || mComplete){
			return;
		}

		mElapsed += deltaTime.asSeconds();

		if (mElapsed >= mAnimation->SecondsPerFrame){
			mFrame++;
			mElapsed -= mAnimation->SecondsPerFrame;
			if (mFrame >= mAnimation->FrameCount){
				if (mAnimation->Loop){
					mFrame = 0;
				} else {
					mFrame = mAnimation->FrameCount - 1;
					mComplete = true;
				}
			}
		}
	}

	void Animator::Render(sf::RenderTarget& target){
		mTarget->setTextureRect(mAnimation->Frames[mFrame]);
		target.draw(*mTarget);
	}

	void Animator::SetTarget(sf::Sprite& target){
		mTarget = &target;
	}

	bool Animator::IsComplete() const{
		return mComplete;
	}

	const Animation& Animator::GetCurrent() const{
		return *mAnimation;
	}
}