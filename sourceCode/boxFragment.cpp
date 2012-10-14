#include "boxFragment.h"
#include "Game.h"
#include "box.h"
#include "images.h"


#include <assert.h>

boxFragment::boxFragment()
{ 
}

boxFragment::boxFragment( sf::Vector2f pos, sf::Time _time,const sf::Texture & _tex)
{ 
	for(int i=0; i<4; i++)
	{
		fragments[i].loadFromTex(_tex);
		assert(fragments[i].IsLoaded());
	}


	fragments[0].SetPosition(pos.x,							pos.y);
	fragments[1].SetPosition(pos.x + Game::GRID_WIDTH/2,	pos.y);
	fragments[2].SetPosition(pos.x + Game::GRID_WIDTH/2,	pos.y + Game::GRID_HEIGHT/2);
	fragments[3].SetPosition(pos.x,							pos.y + Game::GRID_HEIGHT/2);



	sf::IntRect rects[4];

	rects[0] = sf::IntRect	(
						 	0,
							0,
							Game::GRID_WIDTH/2,
							Game::GRID_HEIGHT/2
							);

	rects[1] = sf::IntRect	(
						 	Game::GRID_WIDTH/2,
							0,
							Game::GRID_WIDTH/2,
							Game::GRID_HEIGHT/2
							);

	rects[2] = sf::IntRect	(
						 	Game::GRID_WIDTH/2,
							Game::GRID_HEIGHT/2,
							Game::GRID_WIDTH/2,
							Game::GRID_HEIGHT/2
							);

	rects[3] = sf::IntRect	(
						 	0,
							Game::GRID_HEIGHT/2,
							Game::GRID_WIDTH/2,
							Game::GRID_HEIGHT/2
							);
	
	for(int i=0; i<4; i++)
	{
		fragments[i].GetSprite().setTextureRect(rects[i]);
	}

	

	duration = _time;

	elapsedTime = 0;

	for(int i=0; i<4; i++)
	{
		angles[i] = (float) (std::rand()%45);
	}

	angles[0] += 0;
	angles[1] += 90;
	angles[2] += 180;
	angles[3] += 270;

	const float pi = 3.14159265f;

	for(int i=0; i<4; i++)
	{
		vels[i].x = cos(angles[i]*pi/180)*50.0f;
		vels[i].y = sin(angles[i]*pi/180)*50.0f;
	}
	
}


boxFragment::~boxFragment()
{
}


void boxFragment::Update(float frameTime)
{

	elapsedTime+=frameTime;

	
	//std::cout << elapsedTime << std::endl;

	if ( elapsedTime >= duration.asSeconds() )
	{
		this->setShouldRemove(true);
		return;
	}
	else
	{
		for(int i=0; i<4; i++)
		{
			fragments[i].GetSprite().setColor(sf::Color(255,255,255,(int)floor( 255*(1-elapsedTime/duration.asSeconds()) ) ) );
			fragments[i].GetSprite().move(vels[i]*frameTime);
			fragments[i].GetSprite().rotate(90.0f*frameTime);
		}
	}

	
}

void boxFragment::Draw(sf::RenderWindow & renderWindow)
{
	for(int i=0; i<4; i++)
	{
		fragments[i].Draw(renderWindow);
	}
}