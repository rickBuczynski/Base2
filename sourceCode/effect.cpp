#include "effect.h"
#include "Game.h"
#include "box.h"
#include "images.h"


#include <assert.h>

effect::effect()
{ 
}

effect::effect( float x, float y,  int _frameWidth,int _frameHeight,int _xFrames,int _yFrames, 
				sf::Time _time,sf::Texture & _tex, sf::Vector2f _vel)
{ 
	loadFromTex(_tex);
	assert(IsLoaded());

	

	frameWidth = _frameWidth;
	frameHeight = _frameHeight;
	xFrames = _xFrames;
	yFrames = _yFrames;

	this->SetPosition(x-frameWidth/2,y-frameHeight/2);

	curFrameX = 0;
	curFrameY = 0;

	sf::IntRect frameRect   (
						 	curFrameX*frameWidth,
							curFrameY*frameHeight,
							frameWidth,
							frameHeight
							);
	this->GetSprite().setTextureRect(frameRect);

	effectTime = _time;

	elapsedTime = 0;

	//std::cout << "new effect" << std::endl;

	vel = _vel;
}


effect::~effect()
{
}


void effect::Update(float frameTime)
{

	elapsedTime+=frameTime;

	//std::cout << elapsedTime << std::endl;

	if ( elapsedTime >= effectTime.asSeconds() )
	{
		this->setShouldRemove(true);
		//std::cout << "removeing effect" << std::endl;
		return;
	}
	else
	{
		int currentFrame = (int) floor( xFrames*yFrames*elapsedTime/effectTime.asSeconds() );
	
		curFrameX = currentFrame % xFrames;
		curFrameY = currentFrame / xFrames;

		sf::IntRect frameRect   (
						 		curFrameX*frameWidth,
								curFrameY*frameHeight,
								frameWidth,
								frameHeight
								);
		this->GetSprite().setTextureRect(frameRect);
	}

	this->GetSprite().move(vel);
}
