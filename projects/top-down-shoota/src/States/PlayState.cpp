#include "States/PlayState.hpp"

#include <imgui.h>

#include <math.h>
#include <memory>
#include <iostream>

namespace States{
	PlayState::PlayState(Core::StateContext& context) :
	mContext(context),
	mSoldier(mContext),
	mPlayer(mContext, mSoldier){
		initResources();
		buildScene();
	}

	PlayState::~PlayState(){}

	void PlayState::HandleEvent(const sf::Event& event){
		mPlayer.HandleEvent(event);
	}

	void PlayState::Update(sf::Time deltaTime){
		handlePlayerMouse();
		handlePlayerInput(deltaTime);
		mSoldier.Update(deltaTime);
		updateCameraLerp(deltaTime);
		updateZoom(deltaTime);
	}

	void PlayState::UpdateImGui(sf::Time deltaTime){
		mSoldier.UpdateImGui(deltaTime);
		ImGui::Begin("Play State");
		ImGui::SliderFloat("Zoom Target", &mTargetZoom, 500.f, 2500.f);
		ImGui::SliderFloat("Zoom Smoothing", &mZoomSmoothing, 1.f, 10.f);
		ImGui::SliderFloat("Camera Smoothing", &mCameraSmooting, 1.f, 10.f);
		ImGui::End();
	}

	void PlayState::Render(sf::RenderTarget& target){
		target.setView(mWorldView);
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

	void PlayState::handlePlayerInput(sf::Time deltaTime){
		mSoldier.SetVelocity({0.f, 0.f});
		const float rad = mSoldier.GetAimAngle().asRadians();
		mPlayer.HandleInput(rad);
		mSoldier.Move(deltaTime);
	}

	void PlayState::handlePlayerMouse(){
		const sf::Vector2i pixel = sf::Mouse::getPosition(mContext.mWindow);
		const sf::Vector2f mouseWorld = mContext.mWindow.mapPixelToCoords(pixel, mWorldView);
		const sf::Vector2f pivot = mSoldier.GetCenter();
		const sf::Vector2f delta = mouseWorld - pivot;
		const float rad = std::atan2(delta.y, delta.x);
		mSoldier.SetAimAngle(sf::radians(rad));
	}

	void PlayState::updateCameraLerp(sf::Time deltaTime){
		const float interpolation = 1.f - std::exp(-mCameraSmooting * deltaTime.asSeconds());
		sf::Vector2f center = mWorldView.getCenter();
		mWorldView.setCenter(center + (mSoldier.GetCenter() - center) * interpolation);
	}

	void PlayState::updateZoom(sf::Time deltaTime){
		const float aspect = static_cast<float>(mContext.mWindow.getSize().x) /
							static_cast<float>(mContext.mWindow.getSize().y);
		const float interpolation = 1.f - std::exp(-mCameraSmooting * deltaTime.asSeconds());
		mZoom += (mTargetZoom - mZoom) * interpolation;
		mWorldView.setSize({mZoom * aspect, mZoom});
		mWorldView.setViewport(sf::FloatRect({0.f, 0.f}, {1.f, 1.f}));
	}
}