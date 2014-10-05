#include "armStat.h"

#include <iostream>
#include <fstream>
#include <windows.h>
#include <shlobj.h>

bool armStat::loadFromFile(std::string fileName)
{
	std::ifstream statFile;


	std::string saveDirectory = "assets\\statSheets\\arms\\";
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

			if(typeName == "laser")
				myWeaponType = laser;
			else if(typeName == "gun")
				myWeaponType = gun;
			else if(typeName == "cannon")
				myWeaponType = cannon;
			else if(typeName == "grenadeLauncher")
				myWeaponType = grenadeLauncher;
			else if(typeName == "railGun")
				myWeaponType = railGun;
			else if(typeName == "flameThrower")
				myWeaponType = flameThrower;
		}
		else if (paramater == "playerDamage")
		{
			statFile >> playerDamage;
		}
		else if (paramater == "boxDamage")
		{
			statFile >> boxDamage;
		}
		else if (paramater == "firePeriod")
		{
			statFile >> firePeriod;
		}
		else if (paramater == "projectileSpeed")
		{
			statFile >> projectileSpeed;
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
		else if (paramater == "projName")
		{
			statFile >> projName;
		}
		else if (paramater == "soundName")
		{
			statFile >> soundName;
		}
		else
		{
			statFile.ignore(100000,'\n');
		}
	
	}
	
	statFile.close();
	return true;
}