#include "Core/StateManager.hpp"
#include "States/MainMenu.hpp"
#include "States/SoloMenu.hpp"
#include "States/StartMenu.hpp"

namespace States{
	MainMenu::MainMenu(Core::StateContext& context) : mContext(context){
		initResources();
		buildScene();
		mContext.mResources.Music.Get(Core::Music::Id::MainTheme).play();
	}

	MainMenu::~MainMenu(){
		mContext.mResources.Music.Get(Core::Music::Id::MainTheme).stop();
		mContext.mResources.Music.Unload(Core::Music::Id::MainTheme);
	}

	void MainMenu::Update(sf::Time deltaTime){
		for (int i = 0; i < mButtons.size(); i++){
			UI::Button& button = mButtons[i];
			button.Update();
			if (button.IsMouseHovered(mContext.mWindow)){
				if (button.IsMouseDown(mContext.mWindow)){
					button.ExecuteAction();
				} else {
					button.AutoColor(sf::Color(255,255,255,255));
				}
			} else {
				button.AutoColor(sf::Color(175,175,175,255));
			}
		}
	}

	void MainMenu::Render(sf::RenderTarget& target){
		target.draw(mBackground);
		for (UI::Button& button : mButtons){
			button.Render(target);
		}
	}

	//////////////////////////////////////////////////////////////////////

	void MainMenu::initResources(){
		mContext.mResources.Music.Load(Core::Music::Id::MainTheme, "assets/music/main_theme.ogg");
		mContext.mResources.Music.Get(Core::Music::Id::MainTheme).setLooping(true);
	}

	void MainMenu::buildScene(){

		UI::Button solo_button;
		solo_button.SetContent(mContext, "Solo");
		solo_button.SetPosition({mContext.mWindow.getSize().x/2.f - solo_button.GetTextSize().x/2.f, mContext.mWindow.getSize().y/2.f - solo_button.GetTextSize().y*3.f});
		solo_button.SetAction([this]{
			mContext.Pending.emplace_back([this](Core::StateManager& state){
				state.Replace(std::make_unique<SoloMenu>(mContext));
			});
		});
		mButtons.push_back(std::move(solo_button));

		UI::Button options_button;
		options_button.SetContent(mContext, "Options");
		options_button.SetPosition({mContext.mWindow.getSize().x/2.f - options_button.GetTextSize().x/2.f, solo_button.GetPosition().y + solo_button.GetSize().y + 50.f});
		mButtons.push_back(std::move(options_button));

		UI::Button back_button;
		back_button.SetContent(mContext, "Back");
		back_button.SetPosition({mContext.mWindow.getSize().x/2.f - back_button.GetTextSize().x/2.f, options_button.GetPosition().y + options_button.GetSize().y + 50.f});
		back_button.SetAction([this]{
			mContext.Pending.emplace_back([this](Core::StateManager& state){
				state.Replace(std::make_unique<StartMenu>(mContext));
			});
		});
		mButtons.push_back(std::move(back_button));

		mBackground.setTexture(&mContext.mResources.Textures.Get(Core::Textures::Id::Soldier_Background));
		mBackground.setFillColor(sf::Color(75,75,75,225));
		mBackground.setPosition({0.f,0.f});
		mBackground.setSize(static_cast<sf::Vector2f>(mContext.mWindow.getSize()));
	}
}