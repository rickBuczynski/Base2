#pragma once
#include "mechSpec.h"

class lobbyEvent
{
public:

	// for all events
	enum type { joinRequest, newConnection, playerJoin, playerLeave , gameStart, changeLives, changeSpec, changeGameType, changeTeam, changeFF, changeMap, launchMap, loadedMap, startTranfer };
	type myType;

	// server sends client their player num on connection
	int playerNum;

	int gameTypeNum;

	int teamNum;

	std::string mapName;
	std::string mapHash;

	bool mapReady;

	// path to file you want from home folder
	std::string transferFilePath;

};

