#pragma once
#include "visiblegameobject.h"
#include "player.h"

class grenade : public VisibleGameObject
{
public:

	grenade();
	grenade(float x, float y, player*);
	virtual ~grenade();
	void setVel(float x, float y);
	void setAcc(float x, float y);
	bool checkCollide(float &, float &, float);
	void Update(float elapsedTime);

	void destroyProjectile();

	bool isIntersecting(const VisibleGameObject & vgo);
	bool isIntersecting(const player & ply);

	void generateExplosion();

private:

	int ownerPlayerNum;
	int boxDamage;
	int playerDamage;
	float velX;
	float velY;
	float accX;
	float accY;

	float expRadius;
	float knockForce;
	sf::Time knockTime;

	sf::Time explTime;
};

