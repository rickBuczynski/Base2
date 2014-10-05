#pragma once
#include "visiblegameobject.h"
#include "player.h"
#include "shoulderStat.h"

class missile : public VisibleGameObject
{
public:

	missile();
	missile(float x, float y, player*);

	virtual ~missile();
	void setVel(float x, float y);
	void setAcc(float x, float y);
	bool checkCollide(float &, float &, float);
	void Update(float elapsedTime);

	void generateExplosion();

	void destroyProjectile();

	bool isIntersecting(const VisibleGameObject & vgo);
	bool isIntersecting(const player & ply);


private:

	sf::Vector2f effectPos;

	// for homing missiles
	bool isHoming;
	
	void homeTarget();

	int ownerPlayerNum;
	int boxDamage;
	int playerDamage;
	float velX;
	float velY;
	float accX;
	float accY;

	float expRadius;

	float accScalar;
	float speed;

	float knockForce;
	sf::Time knockTime;

	sf::Time explTime;

	std::string explSound;

	sf::Vector2f getTip();
};

