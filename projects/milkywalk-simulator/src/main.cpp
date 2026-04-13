/*
	“The universe cannot be read until we have learned the language in which it is written.”
		- Galileo Galilei
*/

/*
	render and oval ring of circles to start
	okay we have our oval, now how do we create rings, in varying rotatiion growing in size?
	size can be incremented by iterator?
	rotation can be determined the same way?
*/

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Shader.hpp>

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <iostream>
#include <algorithm>

///

static float randomize(float min, float max){
	return min + static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * (max - min);
}

///

sf::VertexArray particles{};

sf::Shader shader;

int main(){
	srand(time(NULL));
	sf::Clock clock;
	sf::RenderWindow window(sf::VideoMode({1300, 900}), "milkiest way");
	particles = sf::VertexArray(sf::PrimitiveType::Points, 36000);
	shader.loadFromFile("include/milkyboi.frag", sf::Shader::Type::Fragment);
	sf::Vector2f origin = {1300/2.f, 900/2.f};
	sf::RenderStates states;
	states.shader = &shader;
	int ringIndex = 0;
	int particleIndex = 0;
	float rotationStep = 5.f;
	float sizeStep = 5.f;
	float size = 0.f;
	for (int i = 0; i < 100; i++){
		float outerX = static_cast<float>(size);
		float outerY = outerX*.75f;
		float outerRotation = ringIndex * (rotationStep * M_PI / 180.f);
		sf::Vector2f outer = {std::cos(outerRotation), std::sin(outerRotation)};
		for (int j = 0; j < 360; j++){
			float innerRotation = j * (M_PI/180.f);
			sf::Vector2f inner = {outerX * std::cos(innerRotation) + randomize(-50.f, 50.f), outerY * std::sin(innerRotation) + randomize(-50.f, 50.f)};
			//sf::Vector2f inner = {outerX * std::cos(innerRotation), outerY * std::sin(innerRotation)};
			sf::Vector2f rotated = {inner.x * outer.x - inner.y * outer.y, inner.x * outer.y + inner.y * outer.x};
			particles[particleIndex].position = {origin.x + rotated.x, origin.y + rotated.y};
			particles[particleIndex].color = sf::Color::White;
			particleIndex++;
		}
		ringIndex++;
		size+=sizeStep;
	}

	std::cout << ringIndex + particleIndex << '\n';
	float radius = 0.f;
	while (window.isOpen()){
		while (const std::optional event = window.pollEvent()){
			if (event->is<sf::Event::Closed>()){
				window.close();
			}
		}

		// here we can slowly rotate all our particles around center?
		for (int i = 0; i < particles.getVertexCount(); i++){
			auto& particle = particles[i];
			float distanceToCenter = std::hypot(particle.position.x - origin.x, particle.position.y - origin.y);
			if (distanceToCenter > radius){
				radius = distanceToCenter;
			}
		}
		for (int i = 0; i < particles.getVertexCount(); i++){
			auto& particle = particles[i];
			float distanceToCenter = std::hypot(particle.position.x - origin.x, particle.position.y - origin.y);
			// float normalized = distanceToCenter / radius; // rotates it but turns it into a milkshake 0.o
			// float rotation = 1.f / normalized;//* (1.f - normalized);
			float sin = std::sin(0.001f * (M_PI/180.f));
			float cos = std::cos(0.001f * (M_PI/180.f));
			particle.position.x -= origin.x;
			particle.position.y -= origin.y;
			float newx = particle.position.x * cos - particle.position.y * sin;
			float newy = particle.position.x * sin + particle.position.y * cos;
			particle.position.x = newx + origin.x;
			particle.position.y = newy + origin.y;
		}

		shader.setUniform("center", sf::Glsl::Vec2(origin));
		shader.setUniform("radius", radius);

		window.clear(sf::Color::Black);
		// drawing //

		window.draw(particles, states);
		window.display();
	}
	return 0;
}