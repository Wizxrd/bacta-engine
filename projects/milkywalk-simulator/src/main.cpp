/*
	“The universe cannot be read until we have learned the language in which it is written.”
		- Galileo Galilei
*/

/*
	render and oval ring of circles to start
	okay we have our oval, now how do we create rings, in varying rotatiion growing in size?
	size can be incremented by iterator?
	rotation can be determined the same way?



	find distance in whcih intesntity drops

*/

#include "camera.hpp"

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/System/Clock.hpp>

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

struct ParticleData{
	float outerX = 0.f;
	float outerY = 0.f;
	float outerRotation = 0.f;
	float innerRotation = 0.f;
	float speed = 0.f;
	float jitterX = 0.f;
	float jitterY = 0.f;
};

sf::VertexArray particles{};

std::vector<ParticleData> particleData{};

sf::Shader shader;

int main(){
	srand(time(NULL));
	sf::Clock clock;
	sf::RenderWindow window(sf::VideoMode({1920, 1080}), "milkiest way");
	sf::View view;
	Camera camera(window, view);
	particles = sf::VertexArray(sf::PrimitiveType::Points, 36000);
	std::vector<ParticleData> particleData(36000);
	shader.loadFromFile("include/milkyboi.frag", sf::Shader::Type::Fragment);
	sf::Vector2f origin = {1920/2.f, 1080/2.f};
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
			float jitterX = randomize(-25.f, 25.f);
			float jitterY = randomize(-25.f, 25.f);
			sf::Vector2f inner = {outerX * std::cos(innerRotation) + jitterX, outerY * -std::sin(innerRotation) + jitterY};
			//sf::Vector2f inner = {outerX * std::cos(innerRotation), outerY * std::sin(innerRotation)};
			sf::Vector2f rotated = {inner.x * outer.x - inner.y * outer.y, inner.x * outer.y + inner.y * outer.x};
			particles[particleIndex].position = {origin.x + rotated.x, origin.y + rotated.y};
			float t = static_cast<float>(i) / 99.f;
			particleData[particleIndex].outerX = outerX;
			particleData[particleIndex].outerY = outerY;
			particleData[particleIndex].outerRotation = outerRotation;
			particleData[particleIndex].innerRotation = innerRotation;
			particleData[particleIndex].jitterX = jitterX;
			particleData[particleIndex].jitterY = jitterY;
			particleData[particleIndex].speed = 0.01f + std::pow(1.f - t, 7.5f) * 0.5f;
			particleIndex++;
		}
		ringIndex++;
		size+=sizeStep;
	}


	std::cout << ringIndex + particleIndex << '\n';
	float radius = 0.f;
	while (window.isOpen()){
		sf::Time deltaTime = clock.restart();
		float dT = deltaTime.asSeconds();
		while (const std::optional event = window.pollEvent()){
			if (event->is<sf::Event::Closed>()){
				window.close();
			}
			if (event->is<sf::Event::MouseWheelScrolled>()){
				if (event->getIf<sf::Event::MouseWheelScrolled>()->delta > 0){
					camera.Zoom(-25.f);
				} else if (event->getIf<sf::Event::MouseWheelScrolled>()->delta < 0){
					camera.Zoom(25.f);
				}
			}
		}

		// here we can slowly rotate all our particles around center?

		camera.Update(window.getView().getCenter(), deltaTime);

		for (int i = 0; i < particles.getVertexCount(); i++){
			auto& particle = particles[i];
			float distanceToCenter = std::hypot(particle.position.x - origin.x, particle.position.y - origin.y);
			if (distanceToCenter > radius){
				radius = distanceToCenter;
			}
		}

		for (int i = 0; i < particles.getVertexCount(); i++){
			auto& particle = particles[i];
			auto& data = particleData[i];
			data.innerRotation += data.speed * dT;
			float localX = data.outerX * std::cos(data.innerRotation) + data.jitterX;
			float localY = data.outerY * -std::sin(data.innerRotation) + data.jitterY;
			float outerSin = std::sin(data.outerRotation);
			float outerCos = std::cos(data.outerRotation);
			float rotatedX = localX * outerCos - localY * outerSin;
			float rotatedY = localX * outerSin + localY * outerCos;
			particle.position = {origin.x + rotatedX, origin.y + rotatedY};
			float distanceToCenter = std::hypot(rotatedX, rotatedY);
			if (distanceToCenter > radius){
				radius = distanceToCenter;
			}
		}

		shader.setUniform("center", sf::Glsl::Vec2(origin));
		shader.setUniform("radius", radius);

		window.clear(sf::Color::Black);
		window.setView(view);
		window.draw(particles, states);

		window.setView(window.getDefaultView());
		window.display();
	}
	return 0;
}