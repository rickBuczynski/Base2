#pragma once
#include <SFML/Network.hpp>
#include <list>

#include "mechSpec.h"
#include "player.h"
#include "box.h"

class peer
{
public:
	peer();	
	peer(std::string _name, mechSpec _spec, bool _isBot);
	peer(std::string _name, mechSpec _spec );
	peer(std::string _name, mechSpec _spec, sf::IpAddress _address );
	~peer();

	bool upIsPressed;
	bool downIsPressed;
	bool leftIsPressed;
	bool rightIsPressed;


	player* hisPlayer;

	mechSpec spec;
	std::string name;

	sf::IpAddress address;

	enum _team { ffa, red, green, blue };
	_team team;

	bool isBot;

	player* myTarget;

	bool mapReady;

	void runAi();
	bool checkLos(sf::Vector2f initial, sf::Vector2f target);

	sf::Vector2f acquireTarget();
	bool targetAcquired;
	bool targetInLos;
	void sendBotArmAngle();

	void pathFind();
	void aStar(box* start, box* target);


	sf::Thread* path_finder;
	sf::Clock pathTimer;
	bool pathingInProgress;

	std::list<box*> path;

	float pathBottom();
	float pathTop();

	void setDirection();

	enum direction{up,down,left,right,stopH};

	void changeDirection(direction dir);

	box* findSafeBox();
	bool willFall();

	void pursuePlayer();
	void getBackToSafety();
	void getToDistance();

	bool isInLos(player* tarPlay);
	float getDistance(player* tarPlay);
};
