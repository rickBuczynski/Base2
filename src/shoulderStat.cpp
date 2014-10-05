#include "shoulderStat.h"

#include <iostream>
#include <fstream>
#include <windows.h>
#include <shlobj.h>

bool shoulderStat::loadFromFile(std::string fileName)
{
	std::ifstream statFile;


	std::string saveDirectory = "assets\\statSheets\\shoulders\\";
	saveDirectory.append(fileName);
	saveDirectory.append(".txt");

	statFile.open(saveDirectory);

	while(statFile.good())
	{
		std::string paramater;
		statFile >> paramater;

		if (paramater == "myWeaponType")
		{
			std::string typeName;

			statFile >> typeName;

			if(typeName == "missile")
				myWeaponType = missile;
			else if(typeName == "rpg")
				myWeaponType = rpg;
			else if(typeName == "cluster")
				myWeaponType = cluster;
		}
		else if (paramater == "playerDamage")
		{
			statFile >> playerDamage;
		}
		else if (paramater == "boxDamage")
		{
			statFile >> boxDamage;
		}
		else if (paramater == "weight")
		{
			statFile >> weight;
		}
		else if (paramater == "explRadius")
		{
			statFile >> explRadius;
		}
		else if (paramater == "force")
		{
			statFile >> force;
		}
		else if (paramater == "explTime")
		{
			float secs;
			statFile >> secs;
			explTime = sf::seconds(secs);
		}
		else if (paramater == "knockTime")
		{
			float secs;
			statFile >> secs;
			knockTime = sf::seconds(secs);
		}
		else if (paramater == "speed")
		{
			statFile >> speed;
		}
		else if (paramater == "isHoming")
		{
			std::string tOrF;
			statFile >> tOrF;
			if(tOrF == "true")
				isHoming = true;
			else
				isHoming = false;
		}
		else if (paramater == "projName")
		{
			statFile >> projName;
		}
		else if (paramater == "launchSound")
		{
			statFile >> launchSound;
		}
		else if (paramater == "explSound")
		{
			statFile >> explSound;
		}
		else
		{
			statFile.ignore(100000,'\n');
		}
	
	}
	
	statFile.close();
	return true;
}