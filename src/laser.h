#pragma once
#include "visiblegameobject.h"
#include "player.h"

class laser : public VisibleGameObject
{
public:

	laser();
	laser(float x, float y, player*);
	virtual ~laser();
	void setVel(float x, float y);
	void setAcc(float x, float y);
	bool checkCollide();

	void Update(float elapsedTime);
	void Draw(sf::RenderWindow & renderWindow);

	void destroyProjectile();

private:
	sf::Vector2f target;

	int ownerPlayerNum;
	int boxDamage;
	int playerDamage;
	float velX;
	float velY;
};

