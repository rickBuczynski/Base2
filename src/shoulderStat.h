#pragma once
#include <string>
#include <SFML/Window.hpp>


class shoulderStat
{
public:
	
	enum weaponType {missile,rpg,cluster};

	weaponType myWeaponType;

	int boxDamage;
	int playerDamage;
	float explRadius;
	float force;
	float speed;
	sf::Time explTime;
	sf::Time knockTime;

	bool isHoming;

	float weight;

	bool loadFromFile(std::string fileName);

	std::string projName;

	std::string launchSound;
	std::string explSound;

};

