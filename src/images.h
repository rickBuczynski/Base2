#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include <iostream>

class images
{
public:

	class misc
	{
	public:
		static sf::Texture  arrow;
		static sf::Texture  crack1;
		static sf::Texture  crack2;
		static sf::Texture  jetFire;
		static void loadAll();
			
	};
	
	class projectiles
	{
	public:
		static void loadAll();
	};


	class mechParts
	{
	public:
		class legs
		{
			public:
				static void loadAll();
		};

		class bodies
		{
			public:
				static void loadAll();
		};

		class arms
		{
			public:
				static void loadAll();
		};

		class shoulders
		{
			public:
				static void loadAll();
		};

		static void loadAll();
	};

	class explosions
	{
	public:
		static sf::Texture  missile_explosion;
		static sf::Texture  flame;
		static sf::Texture  smoke;
		static void loadAll();
	};


	static std::map<std::string,sf::Texture*> allLegs;
	static std::map<std::string,sf::Texture*> allBodys;
	static std::map<std::string,sf::Texture*> allArms;
	static std::map<std::string,sf::Texture*> allShoulders;

	static std::map<std::string,sf::Texture*> allProjectiles;

	static void loadAll();
};

