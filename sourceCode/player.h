#pragma once
#include "visiblegameobject.h"
#include "mechSpec.h"
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "armStat.h"
#include "shoulderStat.h"
#include "legStat.h"
#include "bodyStat.h"
#include "jetFire.h"
#include "box.h"

class player : public VisibleGameObject
{
public:
	player();
	player(float x, float y,  int _playerNum, int _lives, mechSpec _spec, sf::Color _color);
	virtual ~player();

	void setVel(float x, float y);
	sf::Vector2f getVel();

	void setAcc(float x, float y);
	bool checkCollide(float &, float &, float);
	void Update(float elapsedTime);
	sf::View getView();
	int getPlayerNum();
	sf::Clock cooldownTimer;
	sf::Clock missileTimer;
	sf::Clock respawnTimer;

	sf::Clock laserTicker;

	void takeDamage(int weaponDamage);

	void syncPosition(float x, float y);

	void Draw(sf::RenderWindow & window);

	bool getIsRespawning();
	bool getIsDead();

	void setPartPositions();

	sf::Vector2f getShoulderCenter();

	sf::Vector2f getArmTip();
	float getArmRotation() const;
	void setArmRotation(float angle);
	void setArmFacing(sf::Vector2f pos);

	void kill();

	int getHealth();
	int getLives();

	bool isIntersecting(const VisibleGameObject & vgo, float frameTime, float velX, float velY);
	bool isIntersecting(const player & _player		 , float frameTime, float velX, float velY);
	bool player::isIntersecting(const box & b, float frameTime, float velX, float velY);

	bool containsPoint(sf::Vector2f pos);

	void knockBack(sf::Vector2f direction,sf::Time duration);

	const VisibleGameObject & getBody() const;
	const VisibleGameObject & getArm() const;
	const VisibleGameObject & getShoulder() const;

	armStat getArmStat();
	shoulderStat getShoulderStat();

	bool isLasering;
	bool isCannonCharging;
	sf::Clock chargeTimer;

	sf::Vector2f getCenterMass();
	float gravity;

	void shadeColor(sf::Color);

	float getFuel();

	bool hasRadar;

	bool isGrounded;

	int clearWidth;
	int clearHeight;

	sf::FloatRect getColRect() const;

	bool armColliding();

private:
	sf::FloatRect getBodyLegColRect() const;
	void pushOut();

	jetFire myFire;

	float fuel;

	float maxFuel;
	float jetForce;

	float sheild;

	int maxHealth;
	
	
	bool stillKnocking;

	bool isKnocked;
	sf::Clock knockTimer;
	sf::Time knockDuration;
	sf::Vector2f knockDirection;

	

	float velX;
	float velY;
	float accX;
	float accY;

	int playerNum;

	int lives;

	bool isRespawning;
	bool isDead;

	sf::View view;

	mechSpec spec;

	void setAnimationFrame( float elapsedTime);
	int animationFrameX;
	int animationFrameY;
	float animationTimer;

	VisibleGameObject body;
	VisibleGameObject arm;
	VisibleGameObject shoulder;

	armStat armStat;
	shoulderStat shoulderStat;
	legStat legStat;
	bodyStat bodyStat;

	float weight;
	int health;

	void setParts();
	void setStats();
};

