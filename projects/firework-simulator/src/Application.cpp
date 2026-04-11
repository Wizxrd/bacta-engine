#include "Application.hpp"

#include <SFML/Window/Mouse.hpp>

#include <imgui.h>
#include <imgui-SFML.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <algorithm>

const sf::Time Application::TimePerFrame = sf::seconds(1.f/120.f);

Application::Application()
: mWindow(sf::VideoMode({1300, 900}), "Firework"){
	if (!ImGui::SFML::Init(mWindow)){
		throw std::runtime_error("Unable to init ImGui::SMFL");
	}
}

void Application::Run(){
	sf::Clock clock;
	sf::Time timeSinceLastFrame = sf::Time::Zero;
	while (mWindow.isOpen()){
		sf::Time deltaTime = clock.restart();
		if (deltaTime > sf::seconds(0.25f)){
			deltaTime = sf::seconds(0.25f);
		}
		timeSinceLastFrame += deltaTime;
		processEvent();
		updateImGui(deltaTime);
		while (timeSinceLastFrame >= TimePerFrame){
			timeSinceLastFrame -= TimePerFrame;
			update(TimePerFrame);
		}
		render();
	}
	ImGui::SFML::Shutdown();
}

//////////////////////////////////////////////////////////////////////

void Application::processEvent(){
	while (const std::optional event = mWindow.pollEvent()){
		ImGui::SFML::ProcessEvent(mWindow, *event);
		if (event->is<sf::Event::Closed>()){
			mWindow.close();
		}
		mGame.ProcessEvent(mWindow, *event);
	}
}

void Application::updateImGui(sf::Time deltaTime){
	ImGui::SFML::Update(mWindow, deltaTime);
	mGame.UpdateImGui(deltaTime);
}

void Application::update(sf::Time deltaTime){
	float dT = deltaTime.asSeconds();
	mGame.Update(deltaTime);
}

void Application::render(){
	mWindow.clear(sf::Color::Black);
	mGame.Render(mWindow);
	ImGui::SFML::Render(mWindow);
	mWindow.display();
}