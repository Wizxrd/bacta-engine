#include "States/PlayState.hpp"

#include <imgui.h>

#include <math.h>
#include <memory>
#include <iostream>

namespace States{
	PlayState::PlayState(Core::StateContext& context) :
	mContext(context),
	mSoldier(context),
	mPlayer(mSoldier, context),
	mCamera(context){
		initResources();
		buildScene();
	}

	PlayState::~PlayState(){}

	void PlayState::HandleEvent(const sf::Event& event){
	}

	void PlayState::Update(sf::Time deltaTime){
		mPlayer.Update();
		mSoldier.Update(deltaTime);
		mCamera.Update(mSoldier.GetCenter(), deltaTime);
	}

	void PlayState::UpdateImGui(sf::Time deltaTime){
	}

	void PlayState::Render(sf::RenderTarget& target){
		target.setView(mContext.mView);
		target.draw(mBackground);
		mSoldier.Render(target);
		target.setView(target.getDefaultView());
	}

	//////////////////////////////////////////////////////////////////////

	void PlayState::initResources(){
		mContext.mResources.Textures.Load(Core::Textures::Id::Tile_Stone, "assets/textures/tiles/mixed.png");
	}

	void PlayState::buildScene(){
		sf::Texture& texture = mContext.mResources.Textures.Get(Core::Textures::Id::Tile_Stone);
		texture.setRepeated(true);
		mBackground.setTexture(&texture);
		mBackground.setTextureRect(sf::IntRect({{0, 0}, {10000, 10000}}));
		mBackground.setSize({10000.f, 10000.f});
	}
}