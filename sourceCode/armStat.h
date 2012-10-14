#pragma once
#include <string>
#include <SFML/Window.hpp>

class armStat
{
public:
	
	enum weaponType {gun,laser,cannon,grenadeLauncher,railGun,flameThrower};

	weaponType myWeaponType;

	int playerDamage;
	int boxDamage;
	float firePeriod;
	float projectileSpeed;

	float weight;

	float explRadius;
	float force;
	sf::Time explTime;

	sf::Time knockTime;

	std::string projName;
	std::string soundName;

	bool loadFromFile(std::string fileName);

};

