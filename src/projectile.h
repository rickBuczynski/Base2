#pragma once
#include "visiblegameobject.h"
#include "player.h"

#include "SFML/Audio.hpp"

class projectile : public VisibleGameObject
{
public:

	projectile();
	projectile(float x, float y, player*);
	virtual ~projectile();
	void setVel(float x, float y);
	void setAcc(float x, float y);
	bool checkCollide(float &, float &, float);
	void Update(float elapsedTime);

	void destroyProjectile();

	sf::Sound spawnSound;

private:

	int ownerPlayerNum;
	int boxDamage;
	int playerDamage;
	float velX;
	float velY;
	float accX;
	float accY;
};

