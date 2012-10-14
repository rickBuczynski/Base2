#include "config.h"
#include "utils.h"

#include <direct.h>
#include <iostream>
#include <fstream>
#include <stdio.h>

#include <windows.h>
#include <shlobj.h>

#include <dirent.h>

config::config()
{
}

bool config::loadFromFile()
{

	std::ifstream configFile;

	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return false;

	std::string saveDirectory = path;
	saveDirectory.append("\\Documents\\My Games\\RADgame\\profile\\");
	saveDirectory.append("config.txt");

	configFile.open(saveDirectory);

	resolution.x = 640;
	resolution.y = 480;

	if( !configFile.good() )
		return false;

	while(configFile.good())
	{
		std::string paramater;
		configFile >> paramater;

	
		if (paramater == "resWidth")
		{
			configFile >> resolution.x;
		}
		else if (paramater == "resHeight")
		{
			configFile >> resolution.y;
		}
		else
		{
			configFile.ignore(100000,'\n');
		}
	
	}

	configFile.close();
	return true;

}

bool config::saveToFile()
{
	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return false;

	std::string saveDirectory = path;
	saveDirectory.append("\\Documents\\My Games\\RADgame\\profile\\");
	//std::cout << saveDirectory << std::endl;
	
	std::string command = "mkdir \"";
	command.append(saveDirectory);
	command.append("\"");

	//std::cout << command << std::endl;
	if(!utils::directoryExists(saveDirectory))
		system(command.c_str());

	saveDirectory.append("config.txt");
	//std::cout << saveDirectory << std::endl;

	std::ofstream configFile;

	configFile.open(saveDirectory);

	configFile << "resWidth " << resolution.x << std::endl;
	configFile << "resHeight " << resolution.y << std::endl;

	configFile.close();

	return true;
}