#include "Application.hpp"
#include "MainMenuState.hpp"

#include <cstdio>
#include <optional>

const sf::Time Application::TimePerFrame = sf::seconds(1.f / 60.f);

namespace {

void loadHandgunIdleTextures(GameResources& res) {
	res.handgunIdleFrames.clear();
	for (int i = 0; i < 512; ++i) {
		char path[320];
		std::snprintf(
			path,
			sizeof path,
			"assets/textures/handgun/idle/survivor-idle_handgun_%d.png",
			i);
		auto tex = std::make_unique<sf::Texture>();
		if (!tex->loadFromFile(path)) {
			break;
		}
		res.handgunIdleFrames.push_back(std::move(tex));
	}
}

} // namespace

Application::Application()
: mWindow(sf::VideoMode({1280, 720}), "Top-down shoota")
, mContext(mWindow, mResources) {
	loadResources();
	mStateManager.push(std::make_unique<MainMenuState>(mContext));
}

void Application::loadResources() {
#ifdef _WIN32
	mResources.fonts.tryLoad(Fonts::Id::Main, "C:/Windows/Fonts/segoeui.ttf");
#elif defined(__linux__)
	if (!mResources.fonts.tryLoad(
			Fonts::Id::Main,
			"/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf")) {
		mResources.fonts.tryLoad(
			Fonts::Id::Main, "/usr/share/fonts/TTF/DejaVuSans.ttf");
	}
#endif
	loadHandgunIdleTextures(mResources);
	mResources.soundBuffers.tryLoad(
		SoundBuffers::Id::WeaponAk47Slst,
		"assets/sounds/ak47/weap_ak47_slst_3.wav");
}

void Application::run() {
	sf::Clock clock;
	sf::Time timeSinceLastFrame = sf::Time::Zero;
	while (mWindow.isOpen()) {
		timeSinceLastFrame += clock.restart();
		while (timeSinceLastFrame > TimePerFrame) {
			timeSinceLastFrame -= TimePerFrame;
			processEvents();
			fixedUpdate(TimePerFrame);
		}
		render();
	}
}

void Application::processEvents() {
	while (const std::optional event = mWindow.pollEvent()) {
		if (event->is<sf::Event::Closed>()) {
			mWindow.close();
		}
	}
}

void Application::fixedUpdate(sf::Time deltaTime) {
	if (!mStateManager.empty()) {
		mStateManager.current().update(deltaTime);
	}
	for (auto& action : mContext.pending) {
		action(mStateManager);
	}
	mContext.pending.clear();
}

void Application::render() {
	mWindow.clear(sf::Color(18, 20, 28));
	if (!mStateManager.empty()) {
		mStateManager.current().render(mWindow);
	}
	mWindow.display();
}
