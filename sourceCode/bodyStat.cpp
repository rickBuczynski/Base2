#include "bodyStat.h"

#include <iostream>
#include <fstream>
#include <windows.h>
#include <shlobj.h>

bool bodyStat::loadFromFile(std::string fileName)
{
	std::ifstream statFile;


	std::string saveDirectory = "assets\\statSheets\\bodys\\";
	saveDirectory.append(fileName);
	saveDirectory.append(".txt");

	statFile.open(saveDirectory);

	while(statFile.good())
	{
		std::string paramater;
		statFile >> paramater;

		if (paramater == "health")
		{
			statFile >> health;
		}
		else if (paramater == "weight")
		{
			statFile >> weight;
		}
		else if (paramater == "slots")
		{
			statFile >> slots;
		}
		else if (paramater == "fuel")
		{
			statFile >> fuel;
		}
		else if (paramater == "jetForce")
		{
			statFile >> jetForce;
		}
		else if (paramater == "armX")
		{
			statFile >> armPoint.x;
		}
		else if (paramater == "armY")
		{
			statFile >> armPoint.y;
		}
		else if (paramater == "shoulderX")
		{
			statFile >> shoulderPoint.x;
		}
		else if (paramater == "shoulderY")
		{
			statFile >> shoulderPoint.y;
		}
		else if (paramater == "legOffset")
		{
			statFile >> legOffset;
		}
		else if (paramater == "fireX")
		{
			statFile >> firePoint.x;
		}
		else if (paramater == "fireY")
		{
			statFile >> firePoint.y;
		}
		else
		{
			statFile.ignore(100000,'\n');
		}
	
	}
	
	statFile.close();
	return true;
}