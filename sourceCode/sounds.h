#pragma once
#include "SFML/Audio.hpp"
#include <map>

class sounds
{
public:

	static void loadShots();
	static void loadExplosions();

	static std::map<std::string,sf::SoundBuffer*> shots;
	static std::map<std::string,sf::SoundBuffer*> explosions;

	static void loadAll();
};

