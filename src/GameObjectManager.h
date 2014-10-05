#pragma once
#include "VisibleGameObject.h"
#include <list>


class GameObjectManager
{
public:	
	GameObjectManager();
	~GameObjectManager();

	void Add(VisibleGameObject* gameObject);
	int GetObjectCount() const;

	void DrawAll(sf::RenderWindow& renderWindow);
	void UpdateAll();
	void deleteFlagged();
	sf::Clock frameTimer;

	void deleteAll();

private:
	std::list<VisibleGameObject*> _gameObjects;

	
};