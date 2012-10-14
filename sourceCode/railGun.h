#pragma once
#include "visiblegameobject.h"
#include "player.h"

class railGun : public VisibleGameObject
{
public:

	railGun();
	railGun(float x, float y, player*);
	virtual ~railGun();
	void setVel(float x, float y);
	void setAcc(float x, float y);
	bool checkCollide();

	void Update(float elapsedTime);
	void Draw(sf::RenderWindow & renderWindow);

	void destroyProjectile();

private:

	int ownerPlayerNum;
	int boxDamage;
	int playerDamage;
	float velX;
	float velY;

	sf::Vector2f startPos;
	float startAngle;
	int length;

	sf::Clock fadeTimer;
};

