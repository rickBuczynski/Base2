#include <direct.h>
#include <iostream>
#include <fstream>
#include <stdio.h>

#include <windows.h>
#include <shlobj.h>

#include <dirent.h>

#include "mechSpec.h"
#include "utils.h"

#include "images.h"

mechSpec::mechSpec()
{
	legName = "3d leg";
	bodyName = "3d Body";
	armName = "Gun";
	shoulderName = "Rocket Launcher";
}

bool mechSpec::saveToFile(std::string fileName)
{
	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return false;

	std::string saveDirectory = path;
	saveDirectory.append("\\Documents\\My Games\\RADgame\\saves\\mechs\\");
	//std::cout << saveDirectory << std::endl;
	
	std::string command = "mkdir \"";
	command.append(saveDirectory);
	command.append("\"");

	//std::cout << command << std::endl;
	if(!utils::directoryExists(saveDirectory))
		system(command.c_str());

	saveDirectory.append(fileName);
	saveDirectory.append(".txt");
	//std::cout << saveDirectory << std::endl;

	std::ofstream mechFile;

	mechFile.open(saveDirectory);

	mechFile << legName << std::endl;
	mechFile << bodyName << std::endl;
	mechFile << armName << std::endl;
	mechFile << shoulderName << std::endl;

	mechFile << equipment.size() << std::endl;
	for(std::map<int,std::string>::iterator it = equipment.begin(); it !=  equipment.end(); it++)
	{
		mechFile << it->second << std::endl;
	}


	mechFile.close();

	return true;
}

bool mechSpec::loadFromFile(std::string fileName)
{
	//std::cout << "saving" << std::endl;

	std::ifstream mechFile;

	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return false;

	std::string saveDirectory = path;
	saveDirectory.append("\\Documents\\My Games\\RADgame\\saves\\mechs\\");
	saveDirectory.append(fileName);

	mechFile.open(saveDirectory);

	if (mechFile.good())
	{
		char name[256];

		mechFile.getline(name,256);
		legName = name;

		mechFile.getline(name,256);
		bodyName = name;

		mechFile.getline(name,256);
		armName = name;

		mechFile.getline(name,256);
		shoulderName = name;

		equipment.clear();

		int numEquips;
		mechFile>>numEquips;
		mechFile.ignore(100000,'\n');
		for(int i = 0; i<numEquips; i++)
		{
			std::string equipName;
			mechFile.getline(name,256);
			equipName = name;
			equipment.insert(std::pair<int,std::string>(i,equipName));
			//std::cout << equipName << std::endl;
		}

		mechFile.close();
		return true;
	}
	else
	{
		mechFile.close();
		return false;
	}

}

bool mechSpec::deleteFile(std::string fileName)
{
	//std::cout << "saving" << std::endl;

	std::ifstream mechFile;

	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return false;

	std::string saveDirectory = path;
	saveDirectory.append("\\Documents\\My Games\\RADgame\\saves\\mechs\\");
	saveDirectory.append(fileName);
	
	std::cout << saveDirectory << std::endl;

	if( std::remove( saveDirectory.c_str( ) ) != 0 )
		return false;
	else
		return true;


}
