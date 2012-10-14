#pragma once

#include "SFML/System.hpp"
#include "SFML/Graphics.hpp"
#include <iostream>

class VisibleGameObject
{
public:
	VisibleGameObject();
	virtual ~VisibleGameObject();
	
	void loadFromTex(const sf::Texture&);
	void scale(float x, float y);

	virtual void Load(std::string filename);
	virtual void Draw(sf::RenderWindow & window);
	virtual void Update(float elapsedTime);

	virtual void SetPosition(float x, float y);

	virtual void rotateToFace(float x, float y);
	virtual float getRotation() const;
	virtual void setRotation(float angle);

	virtual sf::Vector2f GetPosition() const;
	virtual float GetWidth() const;
	virtual float GetHeight() const;

	virtual bool IsLoaded() const;

	virtual sf::Rect<float> GetBoundingRect() const;
	virtual sf::ConvexShape getTrueRect() const;

	bool getShouldRemove ();
	void setShouldRemove (bool);

	sf::Sprite& GetSprite();

	// only used for projectiles
	int projNum;
	virtual void destroyProjectile();

private:
	sf::Sprite  _sprite;
	sf::Texture _texture;
	std::string _filename;
	bool _isLoaded;
	bool shouldRemove;
	
};

