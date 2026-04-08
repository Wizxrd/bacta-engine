#include "Core/StateManager.hpp"
#include "States/SoloMenu.hpp"
#include "States/MainMenu.hpp"
#include "States/PlayState.hpp"

namespace States{
	SoloMenu::SoloMenu(Core::StateContext& context) : mContext(context){
		initResources();
		buildScene();
		mContext.mResources.Music.Get(Core::Music::Id::MainTheme).play();
	}

	SoloMenu::~SoloMenu(){
	}

	void SoloMenu::Update(sf::Time deltaTime){
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

	void SoloMenu::Render(sf::RenderTarget& target){
		target.draw(mBackground);
		for (UI::Button& button : mButtons){
			button.Render(target);
		}
	}

	//////////////////////////////////////////////////////////////////////

	void SoloMenu::initResources(){

		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Feet_Idle, "assets/textures/player/feet/idle/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Feet_Run, "assets/textures/player/feet/run/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Feet_Strafe_Left, "assets/textures/player/feet/strafe_left/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Feet_Strafe_Right, "assets/textures/player/feet/strafe_right/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Feet_Walk, "assets/textures/player/feet/walk/sprite_sheet.png");

		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Flashlight_Idle, "assets/textures/player/flashlight/idle/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Flashlight_Melee, "assets/textures/player/flashlight/melee/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Flashlight_Move,"assets/textures/player/flashlight/move/sprite_sheet.png");

		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Handgun_Idle, "assets/textures/player/handgun/idle/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Handgun_Melee, "assets/textures/player/handgun/melee/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Handgun_Move, "assets/textures/player/handgun/move/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Handgun_Reload, "assets/textures/player/handgun/reload/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Handgun_Shoot, "assets/textures/player/handgun/shoot/sprite_sheet.png");

		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Knife_Idle, "assets/textures/player/knife/idle/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Knife_Melee, "assets/textures/player/knife/melee/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Knife_Move, "assets/textures/player/knife/move/sprite_sheet.png");

		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Rifle_Idle, "assets/textures/player/rifle/idle/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Rifle_Melee, "assets/textures/player/rifle/melee/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Rifle_Move, "assets/textures/player/rifle/move/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Rifle_Reload, "assets/textures/player/rifle/reload/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Rifle_Shoot, "assets/textures/player/rifle/shoot/sprite_sheet.png");

		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Shotgun_Idle, "assets/textures/player/shotgun/idle/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Shotgun_Melee, "assets/textures/player/shotgun/melee/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Shotgun_Move, "assets/textures/player/shotgun/move/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Shotgun_Reload, "assets/textures/player/shotgun/reload/sprite_sheet.png");
		mContext.mResources.Textures.Load(Core::Textures::Id::Player_Shotgun_Shoot, "assets/textures/player/shotgun/shoot/sprite_sheet.png");

		mContext.mResources.SoundBuffers.Load(Core::SoundBuffers::Id::Handgun_Fire, "assets/sounds/handgun/fire.wav");
		mContext.mResources.SoundBuffers.Load(Core::SoundBuffers::Id::Handgun_Dryfire, "assets/sounds/handgun/dryfire.wav");
		mContext.mResources.SoundBuffers.Load(Core::SoundBuffers::Id::Handgun_Reload, "assets/sounds/handgun/reload.wav");

		mContext.mResources.SoundBuffers.Load(Core::SoundBuffers::Id::Rifle_Fire, "assets/sounds/rifle/fire.wav");
		mContext.mResources.SoundBuffers.Load(Core::SoundBuffers::Id::Rifle_Dryfire, "assets/sounds/rifle/dryfire.wav");
		mContext.mResources.SoundBuffers.Load(Core::SoundBuffers::Id::Rifle_Reload, "assets/sounds/rifle/reload.wav");

		mContext.mResources.SoundBuffers.Load(Core::SoundBuffers::Id::Shotgun_Fire, "assets/sounds/shotgun/fire_pump.wav");
		mContext.mResources.SoundBuffers.Load(Core::SoundBuffers::Id::Shotgun_Dryfire, "assets/sounds/shotgun/dryfire.wav");
		mContext.mResources.SoundBuffers.Load(Core::SoundBuffers::Id::Shotgun_Reload, "assets/sounds/shotgun/reload.wav");

		mContext.mResources.Music.Load(Core::Music::Id::MainTheme, "assets/music/main_theme.ogg");
		mContext.mResources.Music.Get(Core::Music::Id::MainTheme).setLooping(true);
	}

	void SoloMenu::buildScene(){
		UI::Button resume_game_button;
		resume_game_button.SetContent(mContext, "Resume Game");
		resume_game_button.SetPosition({mContext.mWindow.getSize().x/2.f - resume_game_button.GetTextSize().x/2.f, mContext.mWindow.getSize().y/2.f - resume_game_button.GetTextSize().y*5.f});
		mButtons.push_back(std::move(resume_game_button));

		UI::Button new_game_button;
		new_game_button.SetContent(mContext, "New Game");
		new_game_button.SetPosition({mContext.mWindow.getSize().x/2.f - new_game_button.GetTextSize().x/2.f, resume_game_button.GetPosition().y + resume_game_button.GetSize().y + 50.f});
		new_game_button.SetAction([this]{
			mContext.Pending.emplace_back([this](Core::StateManager& state){
				state.Replace(std::make_unique<PlayState>(mContext));
			});
		});
		mButtons.push_back(std::move(new_game_button));

		UI::Button mission_select_button;
		mission_select_button.SetContent(mContext, "Mission Select");
		mission_select_button.SetPosition({mContext.mWindow.getSize().x/2.f - mission_select_button.GetTextSize().x/2.f, new_game_button.GetPosition().y + new_game_button.GetSize().y + 50.f});
		mButtons.push_back(std::move(mission_select_button));

		UI::Button options_button;
		options_button.SetContent(mContext, "Options");
		options_button.SetPosition({mContext.mWindow.getSize().x/2.f - options_button.GetTextSize().x/2.f, mission_select_button.GetPosition().y + mission_select_button.GetSize().y + 50.f});
		mButtons.push_back(std::move(options_button));

		UI::Button stats_button;
		stats_button.SetContent(mContext, "Stats");
		stats_button.SetPosition({mContext.mWindow.getSize().x/2.f - stats_button.GetTextSize().x/2.f, options_button.GetPosition().y + options_button.GetSize().y + 50.f});
		mButtons.push_back(std::move(stats_button));

		UI::Button back_button;
		back_button.SetContent(mContext, "Back");
		back_button.SetPosition({mContext.mWindow.getSize().x/2.f - back_button.GetTextSize().x/2.f, stats_button.GetPosition().y + stats_button.GetSize().y + 50.f});
		back_button.SetAction([this]{
			mContext.Pending.emplace_back([this](Core::StateManager& state){
				state.Replace(std::make_unique<MainMenu>(mContext));
			});
		});
		mButtons.push_back(std::move(back_button));

		mBackground.setTexture(&mContext.mResources.Textures.Get(Core::Textures::Id::Soldier_Background));
		mBackground.setFillColor(sf::Color(75,75,75,225));
		mBackground.setPosition({0.f,0.f});
		mBackground.setSize(static_cast<sf::Vector2f>(mContext.mWindow.getSize()));
	}
}