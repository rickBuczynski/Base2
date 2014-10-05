#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <math.h>
#include <iostream>

#include "visibleGameObject.h"


VisibleGameObject::VisibleGameObject() :
_isLoaded(false)
{
	setShouldRemove(false);
}


VisibleGameObject::~VisibleGameObject()
{
	setShouldRemove(false);
}

void VisibleGameObject::Load(std::string filename)
{
	if(_texture.loadFromFile(filename) == false)
	{
		_filename =  "";
		_isLoaded = false;
	}
	else
	{
		_filename = filename;
		_sprite.setTexture(_texture);
		_isLoaded = true;
	}
}

void VisibleGameObject::Draw(sf::RenderWindow & renderWindow)
{
	if(_isLoaded)
	{
		renderWindow.draw(_sprite);
	}
}


void VisibleGameObject::Update(float elapsedTime)
{
}

void VisibleGameObject::SetPosition(float x, float y)
{
	if(_isLoaded)
	{
		_sprite.setPosition(x,y);
	}
}

sf::Vector2f VisibleGameObject::GetPosition() const
{
	if(_isLoaded)
	{
		sf::FloatRect boundingRect = this->GetBoundingRect();
		return sf::Vector2f(boundingRect.left,boundingRect.top);
	}
	return sf::Vector2f();
}

float VisibleGameObject::GetHeight() const
{
	sf::IntRect boundingRect;
	boundingRect = _sprite.getTextureRect();
	return (float) boundingRect.height;
}

float VisibleGameObject::GetWidth() const
{
	sf::IntRect boundingRect;
	boundingRect = _sprite.getTextureRect();
	return (float) boundingRect.width;
}

sf::Rect<float> VisibleGameObject::GetBoundingRect() const
{
	return _sprite.getGlobalBounds();
}

sf::ConvexShape VisibleGameObject::getTrueRect() const
{
	/*
	sf::FloatRect boundRect = this->GetBoundingRect();

	sf::ConvexShape polygon;

	polygon.setPointCount(4);

	polygon.setPoint(0, sf::Vector2f(0,					0)						);
	polygon.setPoint(1, sf::Vector2f(boundRect.width,   0)						);
	polygon.setPoint(2, sf::Vector2f(boundRect.width,	boundRect.height)		);
	polygon.setPoint(3, sf::Vector2f(0,					boundRect.height)		);


	polygon.setPosition(this->GetPosition());
	polygon.setRotation(this->getRotation());

	polygon.setOutlineColor(sf::Color::Red);
	polygon.setOutlineThickness(2);
	
	polygon.setFillColor(sf::Color::Transparent);

	return polygon;
	*/

	sf::FloatRect boundRect = _sprite.getLocalBounds();
	//boundRect.left += _sprite.getPosition().x;
	//boundRect.top += _sprite.getPosition().y;

	sf::Vector2f verts [4];

	verts[0].x = boundRect.left;
	verts[0].y = boundRect.top;

	verts[1].x = boundRect.left + boundRect.width;
	verts[1].y = boundRect.top;

	verts[2].x = boundRect.left + boundRect.width;
	verts[2].y = boundRect.top + boundRect.height;

	verts[3].x = boundRect.left;
	verts[3].y = boundRect.top + boundRect.height;


	sf::Transform rotation;
	rotation.rotate(this->getRotation(), _sprite.getOrigin() );

	for (int i = 0; i<4; i++)
		verts[i] = rotation.transformPoint(verts[i]);


	sf::Transform translation;
	translation.translate(_sprite.getPosition() - _sprite.getOrigin());

	for (int i = 0; i<4; i++)
		verts[i] = translation.transformPoint(verts[i]);

	sf::ConvexShape polygon;

	polygon.setPointCount(4);
	for (int i = 0; i<4; i++)
		polygon.setPoint(i,verts[i]);

	//polygon.setPosition(this->GetPosition());

	polygon.setOutlineColor(sf::Color::White);
	polygon.setOutlineThickness(2);
	polygon.setFillColor(sf::Color::Transparent);

	return polygon;
}

sf::Sprite& VisibleGameObject::GetSprite()
{
	return _sprite;
}

bool VisibleGameObject::IsLoaded() const
{
	return _isLoaded;
}

void VisibleGameObject::loadFromTex(const sf::Texture& texture_copy)
{
	_sprite.setTexture(texture_copy);
	_isLoaded = true;
}

void VisibleGameObject::scale(float x, float y)
{
	_sprite.setScale(x,y);
}

bool VisibleGameObject::getShouldRemove ()
{
	return shouldRemove;
}

void VisibleGameObject::setShouldRemove (bool _shouldRemove)
{
	shouldRemove = _shouldRemove;
}

void VisibleGameObject::rotateToFace(float x, float y)
{
	sf::Vector2f position = _sprite.getPosition();
	float angle;
	angle = atan( (y-position.y)/(x-position.x) );

	const float pi = 3.14159265f;

	angle = angle*180/pi;

	if(x < position.x)
		angle+=180;

	if(_isLoaded)
	{
		_sprite.setRotation(angle);
	}
}

float VisibleGameObject::getRotation() const
{
	return _sprite.getRotation();
}

void VisibleGameObject::setRotation(float angle)
{
	_sprite.setRotation(angle);
}

void VisibleGameObject::destroyProjectile()
{
}