#pragma once
#include <string>
#include <list>
#include "SFML/Graphics.hpp"
#include <SFML/Network.hpp>

class utils
{
public:
	static sf::Mutex mutex;
	static bool directoryExists(std::string directory);

	static void normalize(sf::Vector2f & v);

	static void addAxes (std::list<sf::Vector2f> & axes, const sf::ConvexShape & shape);
	static float dotProduct (const sf::Vector2f & u, const sf::Vector2f & v);

	class projection
	{
	public:
		float min;
		float max;

		bool isOverlaping(projection rhs);
	};

	static std::string toString(int number);
	static int toInt(std::string);

	static projection project(const sf::Vector2f & axis, const sf::ConvexShape & shape);

	static float solveQuadratic(float a, float b, float c);

	static float calcSqDistance(sf::Vector2f u,sf::Vector2f v);
};