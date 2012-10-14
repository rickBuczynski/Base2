#pragma once
#include "visiblegameobject.h"
#include "SFML/Audio.hpp"

class jetFire : public VisibleGameObject
{
public:

	bool isShowing;
	float timer;
	int curFrame;


	static const int fireWidth = 50;
	static const int fireHeight = 50;
	static const int totFrames = 13;
	static const int msPerFrame = 10;

	sf::Sound engineSound;

};

