#include <iostream>
#include <SFML/Graphics.hpp>

int main(){
	sf::RenderWindow renderWindow(sf::VideoMode({1300, 900}), "Cube");
	while (renderWindow.isOpen()){
		while (const std::optional event = renderWindow.pollEvent()){
			if (event->is<sf::Event::Closed>()){
				renderWindow.close();
			}
		}

		renderWindow.clear(sf::Color::Black);
		// drawing //
		renderWindow.display();
	}
	return 0;
}