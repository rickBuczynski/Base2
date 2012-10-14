#pragma once
#include <string>
#include <SFML/Window.hpp>

class bodyStat
{
public:

	int health;

	float weight;

	int slots;

	float fuel;
	float jetForce;

	sf::Vector2i armPoint;
	sf::Vector2i shoulderPoint;
	sf::Vector2i firePoint;

	float legOffset;


	bool loadFromFile(std::string fileName);

};

