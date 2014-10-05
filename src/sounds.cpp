#include "sounds.h"
#include <string>
#include <list>
#include <stdio.h>
#include <iostream>

#include "SFML/Audio.hpp"


void sounds::loadShots()
{
	std::list<std::string> names;

	names.push_front("gunShot");
	names.push_front("railLase");
	names.push_front("missileLaunch");
	
	for(std::list<std::string>::iterator itr = names.begin(); itr != names.end(); itr++)
	{
		sf::SoundBuffer* newBuf;
		newBuf = new sf::SoundBuffer;

		std::string directory = "assets/sounds/shots/";
		directory.append(*itr);
		directory.append(".wav");

		std::cout << directory << std::endl;

		newBuf->loadFromFile(directory);

		shots.insert(std::pair<std::string,sf::SoundBuffer*>(*itr,newBuf));
	}
	names.clear();
}

void sounds::loadExplosions()
{
	std::list<std::string> names;

	names.push_front("expl1");
	names.push_front("jetEngine");
	
	for(std::list<std::string>::iterator itr = names.begin(); itr != names.end(); itr++)
	{
		sf::SoundBuffer* newBuf;
		newBuf = new sf::SoundBuffer;

		std::string directory = "assets/sounds/explosions/";
		directory.append(*itr);
		directory.append(".wav");

		std::cout << directory << std::endl;

		newBuf->loadFromFile(directory);

		explosions.insert(std::pair<std::string,sf::SoundBuffer*>(*itr,newBuf));
	}
	names.clear();
}

void sounds::loadAll()
{
	loadShots();
	loadExplosions();
}

std::map<std::string,sf::SoundBuffer*> sounds::shots;
std::map<std::string,sf::SoundBuffer*> sounds::explosions;