#include "box.h"
#include "images.h"
#include "Game.h"
#include "boxFragment.h"

#include <assert.h>

box::box()
{ 
	loadFromTex(Game::mapTileTextures["default_box.png"]);
	assert(IsLoaded());
	durability = 100;

	collide = false;
	isDrawn = false;

	parent = NULL;
	onClosedList = false;
	onOpenList = false;
	shouldOutline = false;
}

box::box(float x, float y)
{ 
	loadFromTex(Game::mapTileTextures["default_box.png"]);
	assert(IsLoaded());
	durability = 100;

	collide = false;
	isDrawn = false;

	SetPosition(x, y);

	parent = NULL;
	onClosedList = false;
	onOpenList = false;
	shouldOutline = false;
}

box::box(float x, float y, sf::Texture &texture)
{ 
	loadFromTex(texture);
	if (IsLoaded() == false)
	{
		loadFromTex(Game::mapTileTextures["default_box.png"]);
		assert(IsLoaded());
	}
	durability = 100;

	collide = false;
	isDrawn = false;

	SetPosition(x, y);

	parent = NULL;
	onClosedList = false;
	onOpenList = false;
	shouldOutline = false;
}

box::~box()
{
}

void box::setCollide(bool _collide)
{
	collide = _collide;
}

bool box::getCollide()
{
	return collide;
}

void box::setIsDrawn(bool _isDrawn)
{
	isDrawn = _isDrawn;
}

bool box::getIsDrawn()
{
	return isDrawn;
}

void box::setTexture(sf::Texture &texture)
{
	loadFromTex(texture);
	if (IsLoaded() == false)
	{
		loadFromTex(Game::mapTileTextures["default_box.png"]);
		assert(IsLoaded());
	}
}

void box::setTextureName(std::string fileName)
{
	textureName = fileName;
}

std::string box::getTextureName()
{
	return textureName;
}

void box::takeDamage(int weaponDamage)
{
	durability-=weaponDamage;
	if(durability <= 0)
	{
		Game::needCleanceReCalc = true;

		isDrawn = false;
		collide = false;
		sf::Time explTime = sf::seconds(0.5f);
		sf::FloatRect rect = this->GetBoundingRect();
		boxFragment* frag = new boxFragment(this->GetPosition(),explTime,*(this->GetSprite().getTexture()) );
		Game::GetGameObjectManager().Add(frag);
	}
}

void box::Draw(sf::RenderWindow & renderWindow)
{
	if( IsLoaded()  )
	{
		renderWindow.draw(GetSprite());
		
		sf::Sprite crack;
		if(durability < 40)
		{
			crack.setTexture(images::misc::crack2);
			crack.setPosition(this->GetPosition());
			renderWindow.draw(crack);
		}
		else if(durability < 70)
		{
			crack.setTexture(images::misc::crack1);
			crack.setPosition(this->GetPosition());
			renderWindow.draw(crack);
		}

	}

	
}

void box::drawOutline(sf::RenderWindow & window,int i, int j)
{
	if(shouldOutline)
	{
		sf::ConvexShape shape;

		shape.setPointCount(4);

		shape.setPoint(0, sf::Vector2f(		0,							0)								);
		shape.setPoint(1, sf::Vector2f(		(float)Game::GRID_WIDTH,	0)								);
		shape.setPoint(2, sf::Vector2f(		(float)Game::GRID_WIDTH,	(float)Game::GRID_HEIGHT)		);
		shape.setPoint(3, sf::Vector2f(		0,							(float)Game::GRID_HEIGHT)		);

		shape.setPosition((float)(i*Game::GRID_WIDTH),(float)(j*Game::GRID_HEIGHT));

		shape.setOutlineColor(sf::Color::Red);
		shape.setOutlineThickness(1);
	
		shape.setFillColor(sf::Color::Transparent);

		window.draw(shape);

		/*
		sf::Text fdisp;
		std::stringstream ftext;
		ftext << this->fScore();
		fdisp.setString( ftext.str() );

		fdisp.setPosition((float)(i*Game::GRID_WIDTH),(float)(j*Game::GRID_HEIGHT));
		fdisp.setCharacterSize(7);
		fdisp.setColor(sf::Color::Black);
		window.draw(fdisp);
		*/
	}
}

int box::fScore()
{
	return gScore + hScore;
}

void box::calcClearance()
{
	for(int i = i_index; i < i_index+maxPlayerWidth; i++)
	{
		if(i>=Game::mapWidth)
		{
			clearances[i-i_index] = 0;
			continue;
		}


		int range;

		if(i-i_index == 0)
		{
			clearances[i-i_index] = maxPlayerHeight;
			range = maxPlayerHeight;
		}
		else
		{
			clearances[i-i_index] = clearances[i-i_index-1];
			range = clearances[i-i_index-1];
		}

		for(int j = j_index; j < j_index + range; j++)
		{
			if(j>=Game::mapHeight)
			{
				clearances[i-i_index] = j - j_index;
				break;
			}
			else if(Game::gameGrid[i][j].getCollide())
			{
				clearances[i-i_index] = j - j_index;
				break;
			}
		}


	}
	
}

int box::getHeightAllowed(int playerWidth)
{
	return clearances[playerWidth-1];
}



void box::drawClearance(sf::RenderWindow & window,int i, int j)
{
	sf::Text disp;
	std::stringstream text;
	text << this->clearances[5];
	disp.setString( text.str() );

	disp.setPosition((float)(i*Game::GRID_WIDTH),(float)(j*Game::GRID_HEIGHT));
	disp.setCharacterSize(10);
	disp.setColor(sf::Color::Black);
	window.draw(disp);
}

bool box::isSafeSpot(int playerWidth,int playerHeight)
{
	if( this->getHeightAllowed(playerWidth) < playerHeight )
		return false;

	for(int i=i_index; i < i_index + playerWidth; i++)
	{
		for(int j=j_index; j<Game::mapHeight; j++)
		{
			if(Game::gameGrid[i][j].getCollide())
				return true;
		}
	}

	return false;
}