#include "Core/Application.hpp"
#include "States/StartMenu.hpp"

#include <imgui.h>
#include <imgui-SFML.h>

namespace Core{

	const sf::Time Application::TimePerFrame = sf::seconds(1.f / 120.f);

	Application::Application()
		: mWindow(sf::VideoMode({1280, 720}), "top down shoota")
		, mContext(mWindow, mResources){
			mContext.mResources.Fonts.Load(Core::Fonts::Id::Default, "assets/fonts/default.ttf");
			mContext.mResources.Fonts.Load(Core::Fonts::Id::Header, "assets/fonts/header.ttf");
			mContext.mResources.Fonts.Load(Core::Fonts::Id::Bold, "assets/fonts/bold.ttf");
			mContext.mResources.Textures.Load(Core::Textures::Id::Soldier_Background, "assets/textures/ui/soldier.png");
			mStateManager.Push(std::make_unique<States::StartMenu>(mContext));
			if (!ImGui::SFML::Init(mWindow)){
				throw std::runtime_error("Unable to int ImGui::SFML");
			}
	}

	void Application::Run(){
		sf::Clock clock;
		sf::Time timeSinceLastFrame = sf::Time::Zero;
		while (mWindow.isOpen()){
			sf::Time deltaTime = clock.restart();
			timeSinceLastFrame += deltaTime;
			processEvents();
			ImGui::SFML::Update(mWindow, deltaTime);
			if (!mStateManager.IsEmpty()){
				mStateManager.GetCurrent().UpdateImGui(deltaTime);
			}
			while (timeSinceLastFrame > TimePerFrame){
				timeSinceLastFrame -= TimePerFrame;
				update(TimePerFrame);
			}
			render();
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Application::processEvents(){
		while (const std::optional event = mWindow.pollEvent()){
			ImGui::SFML::ProcessEvent(mWindow, *event);
			if (event->is<sf::Event::Closed>()) {
				mWindow.close();
			}
		}
	}

	void Application::update(sf::Time deltaTime){
		if (!mStateManager.IsEmpty()){
			mStateManager.GetCurrent().Update(deltaTime);
		}
		for (auto& action : mContext.Pending){
			action(mStateManager);
		}
		mContext.Pending.clear();
	}

	void Application::render(){
		mWindow.clear();
		if (!mStateManager.IsEmpty()){
			mStateManager.GetCurrent().Render(mWindow);
		}
		ImGui::SFML::Render(mWindow);
		mWindow.display();
	}
}