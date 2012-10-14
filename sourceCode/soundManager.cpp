#include "soundManager.h"

void soundManager::Add(sf::Sound* snd)
{
	sounds.push_back(snd);
}

void soundManager::deleteFlagged()
{

	std::list<sf::Sound*>::iterator itr = sounds.begin();
	while(itr != sounds.end())
	{
		// need temp so you can increment before deleting
		// incrementing after deleting gives an error
		std::list<sf::Sound*>::iterator temp;
		temp = itr;
		itr++;
		if(  (*temp)->getStatus() == sf::Sound::Stopped)
		{
			delete (*temp);
			itr = sounds.erase(temp);
		}
	}
}

void soundManager::deleteAll()
{
	std::list<sf::Sound*>::iterator itr = sounds.begin();
	while(itr != sounds.end())
	{
		delete (*itr);
		itr++;
	}
	sounds.clear();
}