#include "States/StartMenu.hpp"
#include "States/MainMenu.hpp"

#include "Core/StateManager.hpp"
#include "Core/Resources.hpp"

#include <math.h>

namespace States{
	StartMenu::StartMenu(Core::StateContext& context) : mContext(context){
		initResources();
		buildScene();
	}

	void StartMenu::Update(sf::Time deltaTime){
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Scancode::Enter)){
			mContext.Pending.emplace_back([this](Core::StateManager& state){
				state.Replace(std::make_unique<MainMenu>(mContext));
			});
		} else { animateText(deltaTime); }
	}

	void StartMenu::Render(sf::RenderTarget& target){
		target.draw(mBackground);
		target.draw(*mText);
	}

	/////////////////////////////////////////////

	void StartMenu::initResources(){
	}

	void StartMenu::buildScene(){
		mText = std::make_unique<sf::Text>(mContext.mResources.Fonts.Get(Core::Fonts::Id::Header));
		mText->setString("Press ENTER");
		mText->setPosition({
			mContext.mWindow.getSize().x/2.f - mText->getGlobalBounds().size.x/2.f,
			mContext.mWindow.getSize().y/2.f - mText->getGlobalBounds().size.y/2.f,
		});

		mBackground.setTexture(&mContext.mResources.Textures.Get(Core::Textures::Id::Soldier_Background));
		mBackground.setFillColor(sf::Color(75,75,75,125));
		mBackground.setPosition({0.f,0.f});
		mBackground.setSize(static_cast<sf::Vector2f>(mContext.mWindow.getSize()));
	}

	void StartMenu::animateText(sf::Time deltaTime){
		mElapsedTime += deltaTime.asSeconds();
		float scale = 1.f + std::sin(mElapsedTime * 2.f) * 0.05f;
		mText->setScale({ scale, scale });
		mText->setPosition({
			mContext.mWindow.getSize().x/2.f - mText->getGlobalBounds().size.x/2.f,
			mContext.mWindow.getSize().y/2.f - mText->getGlobalBounds().size.y/2.f,
		});
	}
}