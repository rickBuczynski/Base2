#pragma once
#include "visiblegameobject.h"
#include "player.h"

class cannon : public VisibleGameObject
{
public:

	cannon();
	cannon(float x, float y, player*);
	virtual ~cannon();
	void setVel(float x, float y);
	void setAcc(float x, float y);
	bool checkCollide(float &, float &, float);
	void Update(float elapsedTime);

	void destroyProjectile();

private:

	int ownerPlayerNum;
	int boxDamage;
	int playerDamage;
	float velX;
	float velY;
	float accX;
	float accY;
};

