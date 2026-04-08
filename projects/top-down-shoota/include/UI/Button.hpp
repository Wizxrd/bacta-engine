#pragma once

#include "Core/Resources.hpp"
#include <Core/StateContext.hpp>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

#include <string>
#include <memory>
#include <functional>

namespace UI{
	class Button{
		public:
			Button() = default;
			~Button() = default;

			Button(const Button&) = delete;
			Button& operator=(const Button&) = delete;
			Button(Button&&) = default;
			Button& operator=(Button&&) = default;

			void Update();
			void Render(sf::RenderTarget& target);
		public:
			void SetPosition(sf::Vector2f position);
			void SetPosition(float x, float y);
			void SetSize(sf::Vector2f size);
			void SetSize(float width, float height);
			void SetTexture(const sf::Texture& texture);
			void AutoColor(sf::Color color);
			void SetBackground(sf::Color color);
			void SetForeground(sf::Color color);
			void SetContent(Core::StateContext& context, const std::string& content);
			void SetCharacterSize(unsigned int size);
			void SetAction(std::function<void()> action);
			void CenterContent();
			sf::Vector2f GetPosition() const;
			sf::Vector2f GetSize() const;
			sf::Vector2f GetTextSize() const;
			bool IsMouseHovered(sf::RenderWindow& window) const;
			bool IsMouseDown(sf::RenderWindow& window) const;
			bool IsMouseReleased(sf::RenderWindow& window) const;
			void ExecuteAction();
		private:
			sf::RectangleShape mShape;
			std::unique_ptr<sf::Text> mText;
			std::function<void()> mAction;
			bool mWasMouseDown = false;
			bool mIsMouseDown = false;
	};
}