#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

class config
{
public:
	config();	

	sf::Vector2i resolution;

	bool loadFromFile();
	bool saveToFile();

};


