#include "UI/Button.hpp"

#include "Core/Resources.hpp"

#include <SFML/Window/Mouse.hpp>

namespace UI{

	void Button::Update(){
		bool pressed = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
		mWasMouseDown = mIsMouseDown;
		mIsMouseDown = pressed;
	}

	void Button::Render(sf::RenderTarget& target){
		target.draw(mShape);
		if (mText){
			target.draw(*mText);
		}
	}

	/////////////////////////////////////////////

	void Button::SetPosition(sf::Vector2f position) {
		mShape.setPosition(position);
		if (mText){
			mText->setPosition(position);
		}
	}

	void Button::SetPosition(float x, float y) {
		mShape.setPosition({x, y});
		if (mText){
			mText->setPosition({x, y});
		}
	}

	void Button::SetSize(sf::Vector2f size) {
		mShape.setSize(size);
	}

	void Button::SetSize(float width, float height) {
		mShape.setSize({width, height});
	}

	void Button::SetTexture(const sf::Texture& texture){
		mShape.setTexture(&texture);
	}

	void Button::AutoColor(sf::Color color){
		if (!mText){
			SetBackground(color);
		} else{
			SetForeground(color);
		}
	}

	void Button::SetBackground(sf::Color color){
		mShape.setFillColor(color);
	}

	void Button::SetForeground(sf::Color color){
		if (mText){
			mText->setFillColor(color);
		}
	}

	void Button::SetContent(Core::StateContext& context, const std::string& content){
		mText = std::make_unique<sf::Text>(context.mResources.Fonts.Get(Core::Fonts::Id::Header));
		mText->setString(content);
	}

	void Button::SetCharacterSize(unsigned int size){
		if (mText){
			mText->setCharacterSize(size);
		}
	}

	void Button::SetAction(std::function<void()> action){
		mAction = action;
	}

	void Button::CenterContent(){
		mText->setPosition({
			(mShape.getPosition().x + mShape.getSize().x/2.f) - mText->getLocalBounds().size.x/2.f,
			(mShape.getPosition().y + mShape.getSize().y/2.f) - mText->getLocalBounds().size.y/2.f,
		});
	}

	sf::Vector2f Button::GetPosition() const {
		return mShape.getPosition();
	}

	sf::Vector2f Button::GetSize() const {
		return mShape.getSize();
	}

	sf::Vector2f Button::GetTextSize() const {
		return mText->getLocalBounds().size;
	}

	bool Button::IsMouseHovered(sf::window& window) const {
		sf::Vector2f mouse = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window));
		if (mText){
			return mText->getGlobalBounds().contains(mouse);
		}
		return mShape.getGlobalBounds().contains(mouse);
	}

	bool Button::IsMouseDown(sf::window& window) const {
		return IsMouseHovered(window) && mIsMouseDown;
	}

	bool Button::IsMouseReleased(sf::window& window) const {
		return IsMouseHovered(window) && !mIsMouseDown && mWasMouseDown;
	}

	void Button::ExecuteAction(){
		if (mAction){
			mAction();
		}
	}
}