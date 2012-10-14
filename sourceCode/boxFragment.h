#pragma once
#include "visiblegameobject.h"
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

class boxFragment : public VisibleGameObject
{
public:
	boxFragment();
	boxFragment( sf::Vector2f pos, sf::Time _time,const sf::Texture & _tex);
	virtual ~boxFragment();

	void Update(float frameTime);
	void boxFragment::Draw(sf::RenderWindow & renderWindow);

private:

	VisibleGameObject fragments[4];
	float angles[4];
	sf::Vector2f vels[4];

	float elapsedTime;

	sf::Time duration;

	int frameWidth;
	int frameHeight;

	int xFrames;
	int yFrames;

	int curFrameX;
	int curFrameY;

	sf::Vector2f vel;
};

