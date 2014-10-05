#include "utils.h"


#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>

bool utils::directoryExists(std::string directory)
{
	DIR *dir;
	dir = opendir (directory.c_str());

	if (dir != NULL) 
	{
		return true;
	} 
	else 
	{
		return false;
	}
}

void utils::addAxes (std::list<sf::Vector2f> & axes, const sf::ConvexShape & shape)
{
	/*
	for(unsigned int i = 0; i<shape.getPointCount(); i++)
	{
		sf::Vector2f point1, point2;

		point1 = shape.getPoint(i);

		if(i == shape.getPointCount()-1)
			point2 = shape.getPoint(0);
		else
			point2 = shape.getPoint(i+1);

		sf::Vector2f side = point2-point1;
		sf::Vector2f normal(-side.y,side.x);

		axes.push_front(normal);
	}
	*/
	sf::Vector2f side1 = shape.getPoint(1)-shape.getPoint(0);
	sf::Vector2f normal1(-side1.y,side1.x);
	axes.push_front(normal1);

	sf::Vector2f side2 = shape.getPoint(2)-shape.getPoint(1);
	sf::Vector2f normal2(-side2.y,side2.x);
	axes.push_front(normal2);

}

utils::projection utils::project(const sf::Vector2f & axis, const sf::ConvexShape & shape)
{
	projection proj;
	proj.min = dotProduct(axis,shape.getPoint(0));
	proj.max = proj.min;
	for (unsigned int i = 1; i < shape.getPointCount(); i++)
	{
		float p = dotProduct(axis,shape.getPoint(i));
		if (p < proj.min) 
			proj.min = p;
		else if (p > proj.max) 
			proj.max = p;
	}
	return proj;
}

float utils::dotProduct (const sf::Vector2f & u, const sf::Vector2f & v)
{
	return u.x * v.x  +  u.y * v.y;
}

bool utils::projection::isOverlaping(projection rhs)
{
	if(this->min > rhs.max)
		return false;
	else if (this->max < rhs.min)
		return false;
	else
		return true;
}

void utils::normalize(sf::Vector2f & v)
{
	float mag = sqrt(v.x*v.x+v.y*v.y);

	v.x = v.x / mag;
	v.y = v.y / mag;
}


std::string utils::toString(int number)
{
   std::stringstream ss;//create a stringstream
   ss << number;//add number to the stream
   return ss.str();//return a string with the contents of the stream
}


int utils::toInt(std::string str)
{
	std::istringstream buffer(str);
	int value;
	buffer >> value; 
	return value;
}


float utils::solveQuadratic(float a, float b, float c)
{
	float sol1 = ( -b + std::sqrt( b*b - 4*a*c) ) / (2*a);
	float sol2 = ( -b - std::sqrt( b*b - 4*a*c) ) / (2*a);

	if(sol1 > 0)
		return sol1;
	else if(sol2 > 0)
		return sol2;
	else
		return 0;
}

float utils::calcSqDistance(sf::Vector2f u,sf::Vector2f v)
{
	float dx = u.x - v.x;
	float dy = u.y - v.y;

	return dx*dx + dy*dy;
}

sf::Mutex utils::mutex;