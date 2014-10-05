#pragma once
#include <string>
#include <map>
#include <SFML/Window.hpp>

class legStat
{
public:
	float moveSpeed;
	int health;

	float weight;

	//int frameWidth; //= 73;
	//int frameH; //= 50;

	sf::Vector2i frameSize;

	int xFrames;// = 6;
	int pixPerFrame; //= 25;

	float waistSize;
	float waistToEdgeDist;

	static const int maxX = 10;
	static const int maxY = 4;
	sf::Vector2f frameOffsets[maxX][maxY];

	bool loadFromFile(std::string fileName);

};

