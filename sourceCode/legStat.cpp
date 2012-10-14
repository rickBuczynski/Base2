#include "legStat.h"

#include <iostream>
#include <fstream>
#include <windows.h>
#include <shlobj.h>

bool legStat::loadFromFile(std::string fileName)
{
	std::ifstream statFile;


	std::string saveDirectory = "assets\\statSheets\\legs\\";
	saveDirectory.append(fileName);
	saveDirectory.append(".txt");

	statFile.open(saveDirectory);


	//initialize frame offesets
	for(int i=0; i<maxX; i++)
	{
		for(int j=0; j<maxY; j++)
		{
			frameOffsets[i][j] = sf::Vector2f(0,0);
		}
	}


	while(statFile.good())
	{
		std::string paramater;
		statFile >> paramater;

		if (paramater == "moveSpeed")
		{
			statFile >> moveSpeed;
		}
		else if (paramater == "health")
		{
			statFile >> health;
		}
		else if (paramater == "weight")
		{
			statFile >> weight;
		}
		else if (paramater == "frameSizeX")
		{
			statFile >> frameSize.x;
		}
		else if (paramater == "frameSizeY")
		{
			statFile >> frameSize.y;
		}
		else if (paramater == "xFrames")
		{
			statFile >> xFrames;
		}
		else if (paramater == "pixPerFrame")
		{
			statFile >> pixPerFrame;
		}
		else if (paramater == "waistSize")
		{
			statFile >> waistSize;
		}
		else if (paramater == "waistToEdgeDist")
		{
			statFile >> waistToEdgeDist;
		}
		else if (paramater == "frameOffset")
		{
			int frameX;
			int frameY;
			sf::Vector2f offset;
			statFile >> frameX;
			statFile >> frameY;
			statFile >> offset.x;
			statFile >> offset.y;

			frameOffsets[frameX][frameY] = offset;
		}
		else
		{
			statFile.ignore(100000,'\n');
		}
	
	}
	
	statFile.close();
	return true;
}