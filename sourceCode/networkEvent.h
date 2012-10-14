#pragma once

class networkEvent
{
public:

	// for all events
	enum type   { moveStart, moveStop, shotFired, missileFired, boxHit, playerHit, sync, 
				  lostFocus , armRotate , clientLeave, serverLeave, gameOver, reJoin,
				  destroyProjectile, knockBack, chargeStart
				};
	type myType;

	// for player movements
	enum moveKey { up, down, left, right };
	moveKey myMoveKey;

	// for shots fired
	float mouseX;
	float mouseY;

	// for box taking hits
	int boxX;
	int boxY;
	int boxDamage;

	// for player taking hits
	int playerNumber;
	int playerDamage;

	// for syncing
	int playerNumToSync;
	float playerX;
	float playerY;
	float velX;
	float velY;

	// for mouse rotation
	float angle;

	// for game over
	int winnerNum;

	// for destroying or making a projectile
	int projNum;

	//for knock back
	sf::Time knockTime;
	sf::Vector2f knockDirecion;
};

