#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include <iostream>
#include <list>


#include "images.h"


void images::loadAll()
{
	projectiles::loadAll();
	mechParts::loadAll();
	explosions::loadAll();
	misc::loadAll();
}

void images::misc::loadAll()
{
	arrow.loadFromFile("assets/misc/arrow.png");
	crack1.loadFromFile("assets/misc/crack1.png");
	crack2.loadFromFile("assets/misc/crack2.png");
	jetFire.loadFromFile("assets/misc/jetFire_13x1_64x64.png");
}

void images::projectiles::loadAll()
{
	std::list<std::string> projNames;

	projNames.push_front("bullet1");
	projNames.push_front("cannonBall1");
	projNames.push_front("grenade1");
	projNames.push_front("missile1");
	projNames.push_front("rpg1");
	projNames.push_front("laser1");
	projNames.push_front("rail1");
	
	for(std::list<std::string>::iterator itr = projNames.begin(); itr != projNames.end(); itr++)
	{
		sf::Texture* newProjectile;
		newProjectile = new sf::Texture;

		std::string directory = "assets/projectiles/";
		directory.append(*itr);
		directory.append(".png");

		std::cout << directory << std::endl;

		newProjectile->loadFromFile(directory);
		allProjectiles.insert(std::pair<std::string,sf::Texture*>(*itr,newProjectile));
	}
	projNames.clear();
}


void images::mechParts::loadAll()
{
	legs::loadAll();
	bodies::loadAll();
	arms::loadAll();
	shoulders::loadAll();
}

void images::mechParts::legs::loadAll()
{
	std::list<std::string> legNames;

	legNames.push_front("3d leg");
	legNames.push_front("4legs");
	
	for(std::list<std::string>::iterator itr = legNames.begin(); itr != legNames.end(); itr++)
	{
		sf::Texture* newLeg;
		newLeg = new sf::Texture;

		std::string directory = "assets/mechs/legs/";
		directory.append(*itr);
		directory.append(".png");

		std::cout << directory << std::endl;

		newLeg->loadFromFile(directory);
		allLegs.insert(std::pair<std::string,sf::Texture*>(*itr,newLeg));
	}
	legNames.clear();
}

void images::mechParts::bodies::loadAll()
{
	std::list<std::string> bodyNames;

	bodyNames.push_front("Green Body");
	bodyNames.push_front("Grey Body");
	bodyNames.push_front("Red Body");
	bodyNames.push_front("Whelp Body");
	bodyNames.push_front("3d Body");
	
	for(std::list<std::string>::iterator itr = bodyNames.begin(); itr != bodyNames.end(); itr++)
	{
		sf::Texture* newBody;
		newBody = new sf::Texture;

		std::string directory = "assets/mechs/bodys/";
		directory.append(*itr);
		directory.append(".png");

		std::cout << directory << std::endl;

		newBody->loadFromFile(directory);
		allBodys.insert(std::pair<std::string,sf::Texture*>(*itr,newBody));
	}
	bodyNames.clear();
}

void images::mechParts::arms::loadAll()
{
	std::list<std::string> armNames;

	armNames.push_front("Gun");
	armNames.push_front("Laser");
	armNames.push_front("Cannon");
	armNames.push_front("Grenade Launcher");
	armNames.push_front("Rail Gun");
	
	for(std::list<std::string>::iterator itr = armNames.begin(); itr != armNames.end(); itr++)
	{
		sf::Texture* newArm;
		newArm = new sf::Texture;

		std::string directory = "assets/mechs/arms/";
		directory.append(*itr);
		directory.append(".png");

		std::cout << directory << std::endl;

		newArm->loadFromFile(directory);
		allArms.insert(std::pair<std::string,sf::Texture*>(*itr,newArm));
	}
	armNames.clear();
}

void images::mechParts::shoulders::loadAll()
{
	std::list<std::string> shoulderNames;

	shoulderNames.push_front("Homing Launcher");
	shoulderNames.push_front("Rocket Launcher");
	shoulderNames.push_front("RPG");
	shoulderNames.push_front("Cluster Launcher");
	
	for(std::list<std::string>::iterator itr = shoulderNames.begin(); itr != shoulderNames.end(); itr++)
	{
		sf::Texture* newShoulder;
		newShoulder = new sf::Texture;

		std::string directory = "assets/mechs/shoulders/";
		directory.append(*itr);
		directory.append(".png");

		std::cout << directory << std::endl;

		newShoulder->loadFromFile(directory);
		allShoulders.insert(std::pair<std::string,sf::Texture*>(*itr,newShoulder));
	}
	shoulderNames.clear();
}

void images::explosions::loadAll()
{
	missile_explosion.loadFromFile("assets/explosions/explosion0_8x8_128x128.png");
	flame.loadFromFile("assets/explosions/flame_8x8_32x32.png");
	smoke.loadFromFile("assets/explosions/smoke_4x1_8x8.png");
}

sf::Texture images::misc::arrow;
sf::Texture images::misc::crack1;
sf::Texture images::misc::crack2;
sf::Texture images::misc::jetFire;


sf::Texture images::explosions::missile_explosion;
sf::Texture images::explosions::flame;
sf::Texture images::explosions::smoke;


std::map<std::string,sf::Texture*> images::allLegs;
std::map<std::string,sf::Texture*> images::allBodys;
std::map<std::string,sf::Texture*> images::allArms;
std::map<std::string,sf::Texture*> images::allShoulders;

std::map<std::string,sf::Texture*> images::allProjectiles;

