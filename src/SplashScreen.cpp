#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "SplashScreen.h"
#include "Game.h"


bool SplashScreen::Show(sf::RenderWindow & renderWindow)
{
	sf::Texture splashImg;
	if(splashImg.loadFromFile("assets/SplashScreen.png") != true)
	{
		return true;
	}

	sf::Sprite sprite(splashImg);
	sprite.setPosition((float)(Game::myConfig.resolution.x-splashImg.getSize().x)/2,(float)(Game::myConfig.resolution.y-splashImg.getSize().y)/2);
	renderWindow.draw(sprite);
	renderWindow.display();

	sf::Event currentEvent;
	while(true)
	{
		renderWindow.waitEvent(currentEvent);

		if( currentEvent.type == sf::Event::Closed ) 
			return true;

		if(currentEvent.type == sf::Event::LostFocus)
			Game::isFocused = false;

		else if(currentEvent.type == sf::Event::GainedFocus)
			Game::isFocused = true;

		if(currentEvent.type == sf::Event::KeyPressed 
			|| currentEvent.type == sf::Event::MouseButtonPressed
			|| currentEvent.type == sf::Event::Closed )
		{
			return false;
		}





	}
}