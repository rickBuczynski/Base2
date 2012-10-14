#pragma once
#include "visiblegameobject.h"


class box : public VisibleGameObject
{

public:
	const static int maxPlayerWidth = 6;
	const static int maxPlayerHeight = 8;

	sf::Clock laserTicker;

	box();
	box(float x, float y);
	box(float x, float y, sf::Texture &texture);
	virtual ~box();

	void setCollide(bool);
	bool getCollide();

	void setIsDrawn(bool);
	bool getIsDrawn();

	void setTexture(sf::Texture &texture);
	void setTextureName(std::string fileName);
	std::string getTextureName();

	void takeDamage(int weaponDamage);

	void Draw(sf::RenderWindow & window);
	void drawOutline(sf::RenderWindow & window,int i, int j);

	// for finding paths
	bool shouldOutline;
	bool isCurrent;

	int i_index;
	int j_index;

	int gScore;
	int hScore;
	int fScore();
	box* parent;

	bool onClosedList;
	bool onOpenList;

	void drawClearance(sf::RenderWindow & window,int i, int j);
	
	// the index is the width of the player minus 1
	// the value at that index is how tall the player is allowed to be
	
	void calcClearance();

	int getHeightAllowed(int playerWidth);
	
	bool isSafeSpot(int playerWidth,int playerHeight);

	// used when trying to flee
	// is the distance from the box to that player
	float sqDist;

private:
	bool collide;
	bool isDrawn;
	int durability;
	int clearances[maxPlayerWidth];
	std::string textureName;

	


};

