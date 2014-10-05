#pragma once
#include <list>
#include "SFML/Audio.hpp"

class soundManager
{
public:	
	void Add(sf::Sound* snd);

	void deleteFlagged();

	void deleteAll();

private:
	std::list<sf::Sound*> sounds;

	
};