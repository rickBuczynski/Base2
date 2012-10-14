#include "GameObjectManager.h"
#include "Game.h"
#include "utils.h"

GameObjectManager::GameObjectManager()
{
}

// may need to add something here
GameObjectManager::~GameObjectManager()
{
}

void GameObjectManager::Add(VisibleGameObject* gameObject)
{
	_gameObjects.push_back(gameObject);
}



int GameObjectManager::GetObjectCount() const
{
	return _gameObjects.size();
}


void GameObjectManager::DrawAll(sf::RenderWindow& renderWindow)
{

	std::list<VisibleGameObject*>::const_iterator itr = _gameObjects.begin();
	while(itr != _gameObjects.end())
	{
		(*itr)->Draw(renderWindow);
		itr++;
	}
}

void GameObjectManager::UpdateAll()
{
	std::list<VisibleGameObject*>::const_iterator itr = _gameObjects.begin();
	float timeDelta;
	timeDelta = frameTimer.getElapsedTime().asSeconds();
	// prevent huge jumps if window is dragged or resized
	if (timeDelta > 0.1f)
		timeDelta = 0.1f;

	frameTimer.restart();

	while(itr != _gameObjects.end())
	{
		(*itr)->Update(timeDelta);
		itr++;
	}
	
}



void GameObjectManager::deleteFlagged()
{
	utils::mutex.lock();

	std::list<VisibleGameObject*>::iterator itr = _gameObjects.begin();
	while(itr != _gameObjects.end())
	{
		// need temp so you can increment before deleting
		// incrementing after deleting gives an error
		std::list<VisibleGameObject*>::iterator temp;
		temp = itr;
		itr++;
		if(  (*temp)->getShouldRemove() )
		{
			delete (*temp);
			itr = _gameObjects.erase(temp);
		}
	}

	utils::mutex.unlock();
}



void GameObjectManager::deleteAll()
{
	std::list<VisibleGameObject*>::iterator itr = _gameObjects.begin();
	while(itr != _gameObjects.end())
	{
		// need temp so you can increment before deleting
		// incrementing after deleting gives an error
		std::list<VisibleGameObject*>::iterator temp;
		temp = itr;
		itr++;
		delete (*temp);
		itr = _gameObjects.erase(temp);
	}
	_gameObjects.clear();
}