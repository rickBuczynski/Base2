#pragma once
#include "visiblegameobject.h"
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

class effect : public VisibleGameObject
{
public:
	effect();
	effect( float x, float y,  int _frameWidth,int _frameHeight,
			int _xFrames,int _yFrames, sf::Time _time,sf::Texture & _tex, sf::Vector2f _vel);
	virtual ~effect();

	void Update(float frameTime);
	
private:
	float elapsedTime;

	sf::Time effectTime;

	int frameWidth;
	int frameHeight;

	int xFrames;
	int yFrames;

	int curFrameX;
	int curFrameY;

	sf::Vector2f vel;
};

