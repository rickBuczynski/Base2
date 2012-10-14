#include "Game.h"
#include "visibleGameObject.h"
#include "box.h"
#include "player.h"
#include "images.h"
#include "packet.h"
#include "multiplayer.h"
#include "projectile.h"
#include "networkEvent.h"
#include "mechSpec.h"
#include "SplashScreen.h"
#include "MainMenu.h"
#include "utils.h"
#include "messageWindow.h"
#include "missile.h"
#include "laser.h"
#include "cannon.h"
#include "grenade.h"
#include "railGun.h"
#include "flame.h"
#include "sounds.h"
#include "SHATools.h"

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Network.hpp>

#include <windows.h>
#include <shlobj.h>
#include <dirent.h>

#include <fstream>
#include <iostream>
#include <sstream>

void Game::Start(void)
{
	// defaults
	Game::lives = 1;
	Game::myGameType = ffa;
	Game::ffIsOn = false;
	prevAngle = 0;
	selectedMap = "realMapFile";

	musicPlaying = false;

	mapIsInMemory = false;
	
	isReHosting = false;
	isReJoining = false;

	needCleanceReCalc = false;
	reCalcingClearance = false;

	readName();

	if(_gameState != Uninitialized)
		return;

	// loading tiles
	images::loadAll();
	sounds::loadAll();


	Game::myBuildMenu = new buildMenu;

	_mainWindow.create(sf::VideoMode(myConfig.resolution.x,myConfig.resolution.y,32),"RaDgameTest!");
	isFocused = true;
	
	_gameState= Game::ShowingSplash;
	while(!IsExiting())
	{
		GameLoop();
	}
	std::cout << "about to clean" << std::endl;
	Game::cleanUp();
	std::cout << "about to close" << std::endl;
	_mainWindow.close();

	delete Game::myBuildMenu;
}

bool Game::IsExiting()
{
	if(_gameState == Game::Exiting) 
		return true;
	else 
		return false;
}


sf::RenderWindow& Game::GetWindow()
{
	return _mainWindow;
}


GameObjectManager& Game::GetGameObjectManager()
{
	return _gameObjectManager;
}


void Game::GameLoop()
{

	bool eventWasReceived;
	sf::Event currentEvent;
	eventWasReceived = _mainWindow.pollEvent(currentEvent);

	mySoundManager.deleteFlagged();

	switch(_gameState)
	{
	case Game::ShowingMenu:
		ShowMenu();
		break;
	case Game::ShowingSplash:
		ShowSplashScreen();
		break;
	case Game::Playing:
		if (eventWasReceived)
			handleMultiplayerEvent(currentEvent);

		playMultiplayer();

		break;
	case Game::GameOver:
		showGameOverScreen(currentEvent);
		break;
	case Game::mechBuilding:
		buildMenu::result result;
		result = myBuildMenu->Show(_mainWindow);
		switch (result)
		{
		case buildMenu::quit:
			_gameState = Exiting;
			break;
		case buildMenu::done:
			mainMenu.restart();
			_gameState = ShowingMenu;
			break;
		default:
			break;
		}
		break;
	default:
		break;

	}

}

void Game::ShowSplashScreen()
{
	SplashScreen splashScreen;
	bool exitFromSplash = splashScreen.Show(_mainWindow);
	
	if(exitFromSplash)
		_gameState = Game::Exiting;
	else
		_gameState = Game::ShowingMenu;
}

void Game::ShowMenu()
{

	MainMenu::MenuResult result = mainMenu.Show(_mainWindow);
	switch(result)
	{
		case MainMenu::Exit:
			_gameState = Exiting;
			break;
		case MainMenu::Join:
			multiplayer::StartClient(mainMenu.ip);
			

			if(multiplayer::quittingFromLobby)
			{
				_gameState = Exiting;
			}
			else if ( multiplayer::getGameStarted() )
			{
				_gameState = Playing;
				setupMultiplayer();
			}
			else
			{
				_gameState = ShowingMenu;
			}

			break;
		case MainMenu::Host:
			multiplayer::StartServer();

			if(multiplayer::quittingFromLobby)
			{
				std::cout << "quit from lobby" << std::endl;
				_gameState = Exiting;
			}
			else if ( multiplayer::getGameStarted() )
			{
				setupMultiplayer();
				_gameState = Playing;
			}
			else
			{
				_gameState = ShowingMenu;
			}

			break;
		case MainMenu::Build:
			_gameState = mechBuilding;
			myBuildMenu->start();
			break;
		default:
			break;
	}

}

void Game::setupMultiplayer()
{
	projectileCount = 0;

	Game::allProjectiles.clear();

	char path[ MAX_PATH ];
	SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path );

	std::string mapDir = path;
	mapDir.append("\\Documents\\My Games\\RADgame\\maps\\");

	mapDir.append(selectedMap);
	mapDir.append(".txt");
	mapLoader(mapDir);

	//curMusic = new sf::Music;
	//curMusic->openFromFile("assets/music/Decimeter-Stand_Alone_Complex_(Second_Gig_Remix).ogg");
	//curMusic->setLoop(true);
	//curMusic->play();
	//musicPlaying = true;

	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it != multiplayer::allPeers.end(); ++it)
	{
		sf::Color color;
		switch(it->second->team)
		{
		case peer::ffa:
			color = sf::Color::White;
			break;
		case peer::red:
			color = sf::Color::Red;
			break;
		case peer::green:
			color = sf::Color::Green;
			break;
		case peer::blue:
			color = sf::Color::Blue;
			break;
		}

		it->second->hisPlayer = new player((float)(it->first+5)*4.f*GRID_WIDTH,4*GRID_HEIGHT,it->first,Game::lives,it->second->spec,color);
		_gameObjectManager.Add(it->second->hisPlayer);
	}

	myPlayer = multiplayer::allPeers.find(multiplayer::myPlayerNum)->second->hisPlayer;

	if ( multiplayer::_peerState == multiplayer::server)
	{
		network_server_listen.launch();
		network_server_send_sync.launch();
	}
	else if ( multiplayer::_peerState == multiplayer::client)
		network_client_listen.launch();

	return;
}

void Game::BoxManager () 
{

		int i_left;
		int j_top;
		int i_right;
		int j_bottom;

		sf::Vector2f viewPos = myPlayer->getView().getCenter();
		sf::Vector2f viewSize = myPlayer->getView().getSize();

		i_left =   (int) floor(  (viewPos.x - viewSize.x/2) / Game::GRID_WIDTH  );
		j_top =    (int) floor(  (viewPos.y - viewSize.y/2) / Game::GRID_HEIGHT);
		i_right =  (int) floor(  (viewPos.x + viewSize.x/2) / Game::GRID_WIDTH);
		j_bottom = (int) floor(  (viewPos.y + viewSize.y/2) / Game::GRID_HEIGHT);


		if (i_left < 0)
			i_left = 0;
		if (i_left >= Game::mapWidth)
			i_left = Game::mapWidth-1;

		if (i_right < 0)
			i_right = 0;
		if (i_right >= Game::mapWidth)
			i_right = Game::mapWidth-1;

		if (j_top < 0)
			j_top = 0;
		if (j_top >= Game::mapHeight)
			j_top = Game::mapHeight-1;

		if (j_bottom < 0)
			j_bottom = 0;
		if (j_bottom >= Game::mapHeight)
			j_bottom = Game::mapHeight-1;

		int i;
		int j;
		for (i = i_left; i <= i_right ; i++)
		{

			for (j = j_top; j <= j_bottom ; j++)
			{
				if(gameGrid[i][j].getIsDrawn())
				{
					gameGrid[i][j].Draw(_mainWindow);
				}
			}

		}

		for (i = i_left; i <= i_right ; i++)
		{

			for (j = j_top; j <= j_bottom ; j++)
			{
				if(gameGrid[i][j].shouldOutline)
				{
					gameGrid[i][j].drawOutline(_mainWindow,i,j);
				}

				//gameGrid[i][j].drawClearance(_mainWindow,i,j);

			}

		}

}

void Game::mapDestroyer()
{
	for (int i = 0; i < mapWidth; i++)
		delete[] gameGrid[i];

	delete[] gameGrid;

	mapIsInMemory = false;
}

void Game::mapLoader (std::string mapName) 
{

	background.Load("assets/backgrounds/sky1.png");
	background.scale(Game::myConfig.resolution.x/1920.0f,Game::myConfig.resolution.y/1200.0f);

	midground.Load("assets/midgrounds/trees.png");
	
	std::map<std::string, std::string> renamedTextures;
	renamedTextures.clear();

	Game::mapTileTextures.clear();
	char path[ MAX_PATH ];
	SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path );
	std::string texDir = path;
	texDir.append("\\Documents\\My Games\\RADgame\\tiles\\");

	sf::Texture empty_box;
	empty_box.loadFromFile( texDir + "empty_box.png");
	Game::mapTileTextures.insert( std::pair<std::string, sf::Texture>( "empty_box.png", empty_box ) );

	sf::Texture default_box;
	default_box.loadFromFile( texDir + "default_box.png");
	Game::mapTileTextures.insert( std::pair<std::string, sf::Texture>( "default_box.png", default_box ) );

	sf::Texture dirt_box;
	dirt_box.loadFromFile( texDir + "dirt_box.png");
	Game::mapTileTextures.insert( std::pair<std::string, sf::Texture>( "dirt_box.png", dirt_box ) );

	sf::Texture moon_box;
	moon_box.loadFromFile( texDir + "moon_box.png");
	Game::mapTileTextures.insert( std::pair<std::string, sf::Texture>( "moon_box.png", moon_box ) );

	int buffer;
	std::ifstream mapFile;
	mapFile.open(mapName);

	//std::string sha512;
	//sha512 = SHATools::getSha512Hash(mapName, true); 
	//std::cout << "The SHA-512 of the map file is: " << sha512 << std::endl;

	if (mapFile.is_open())
	{
		if ( mapFile.good() )
		{
			mapFile >> buffer;
			if (buffer == 0)
			{
				mapFile >> mapWidth;
				gameGrid = new box*[mapWidth];
				mapFile >> mapHeight;
				for (int i = 0; i < mapWidth; i++)
					gameGrid[i] = new box[mapHeight];
			}
			else
			{
				std::cout << "Error getting size of map." << std::endl;
				std::cout << "Buffer value was: " << buffer << std::endl;
			}
		}
		while ( mapFile.good() )
		{
			mapFile >> buffer;
			if (buffer == 1)
			{
				std::string textureName; 
				std::getline(mapFile, textureName);
				textureName.erase(0,1);
				sf::Texture tempTexture;
				std::cout << "textureName: " << textureName << std::endl;
				if (textureName.size() > 13 && textureName.substr( textureName.size() - 13, 6) == "_COPY_")
				{
					char path[ MAX_PATH ];
					SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path );
					std::string newTexDir = path;
					newTexDir.append("\\Documents\\My Games\\RADgame\\maps\\");
					tempTexture.loadFromFile(newTexDir + textureName);
					int copyNumber = 0;
					std::string textureNameChecking = textureName;
					textureNameChecking.erase( textureName.size() - 13, 9);
					Game::insertTileTexture( renamedTextures, textureName, textureNameChecking, copyNumber, tempTexture );
				}
				else
				{

				}
			}
			else if (buffer == 2)
			{
				int x;
				mapFile >> x;
				int y;
				mapFile >> y;
				std::string textureName; 
				std::getline(mapFile, textureName);
				textureName.erase(0,1);
				gameGrid[x][y].setIsDrawn(true);
				gameGrid[x][y].setCollide(true);
				gameGrid[x][y].SetPosition(GRID_WIDTH* (float) x, GRID_HEIGHT* (float) y);
				if (renamedTextures.find(textureName) == renamedTextures.end())
				{
					gameGrid[x][y].setTexture(Game::mapTileTextures[textureName]);
					gameGrid[x][y].setTextureName(textureName);
				}
				else
				{
					renamedTextures.find(textureName)->second;
					gameGrid[x][y].setTexture(Game::mapTileTextures.find(renamedTextures.find(textureName)->second)->second);
					gameGrid[x][y].setTextureName(renamedTextures.find(textureName)->second);
				}
			}
			else
			{
				std::cout << "Error determining line type" << std::endl;
				std::cout << "Buffer value was: " << buffer << std::endl;
			}
		}
		mapFile.close();
	}

	for(int i = 0; i<mapWidth; i++)
	{
		for(int j = 0; j<mapHeight; j++)
		{
			Game::gameGrid[i][j].i_index = i;
			Game::gameGrid[i][j].j_index = j;
			Game::gameGrid[i][j].calcClearance();
		}
	}

	mapIsInMemory = true;
}

void Game::insertTileTexture ( std::map<std::string, std::string> &renamedTextures, std::string &textureName, std::string &textureNameChecking, int &copyNumber, sf::Texture &tempTexture )
{
	std::cout << textureNameChecking << " is being attempted." << std::endl;
	std::pair<std::map<std::string, sf::Texture>::iterator,bool> insertCheck;
	insertCheck = Game::mapTileTextures.insert( std::pair<std::string, sf::Texture>( textureNameChecking, tempTexture ) );
	if (insertCheck.second == false)
	{
		std::cout << textureNameChecking << " already existed in toolboxTileTextures." << std::endl;

		char path[ MAX_PATH ];
		SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path );
		std::string newTexDir = path;
		newTexDir.append("\\Documents\\My Games\\RADgame\\maps\\");
		std::string oldTexDir = path;
		oldTexDir.append("\\Documents\\My Games\\RADgame\\tiles\\");

		std::string sha512MapDirectory;
		sha512MapDirectory = SHATools::getSha512Hash( newTexDir + textureName, true ); 
		std::cout << "The SHA-512 of the map directory's texture is: " << sha512MapDirectory << std::endl;

		std::string sha512AlreadyExisting;
		sha512AlreadyExisting = SHATools::getSha512Hash( oldTexDir + textureNameChecking, true ); 
		std::cout << "The SHA-512 of the already existing texture is: " << sha512AlreadyExisting << std::endl;

		if (sha512MapDirectory.compare(sha512AlreadyExisting) == 0)
		{
			std::cout << "They are equal." << std::endl;
			if (textureName.compare(textureNameChecking) == 0)
			{
				//Okay to load map from existing tile of same name
				std::cout << "Loading tile from existing tile of same name: " << textureName << std::endl;
			}
			else
			{
				//Okay to load map from existing tile of checked name
				std::cout << "Loading tile from existing tile of checked name: " << textureNameChecking << std::endl;
				renamedTextures.insert( std::pair<std::string, std::string>( textureName, textureNameChecking ) );
			}
		}
		else
		{
			std::cout << "They are not equal." << std::endl;
			copyNumber++;
			if ( copyNumber == 1 )
			{
				textureNameChecking.insert( textureNameChecking.size()-4, "_COPY_001" );
			}
			else if ( copyNumber < 10 )
			{
				std::stringstream tempStream;
				tempStream << "00" << copyNumber;
				std::string tempString = tempStream.str();
				std::cout << "tempString: " << tempString << std::endl;
				textureNameChecking.replace( textureNameChecking.size()-7, 3, tempString );
			}
			else if ( copyNumber >= 10 && copyNumber < 100 )
			{
				textureNameChecking.replace( textureNameChecking.size()-7, 3, "0" + copyNumber );
			}
			else if ( copyNumber >= 100 && copyNumber < 1000 )
			{
				textureNameChecking.replace( textureNameChecking.size()-7, 3, "" + copyNumber );
			}
			else
			{
				std::cout << "Reached maximum number of copies. Overwrite last here." << std::endl;
			}
			Game::insertTileTexture( renamedTextures, textureName, textureNameChecking, copyNumber, tempTexture );
		}
	}
	else
	{
		std::cout << textureNameChecking << " did not already exist." << std::endl;
	}
}

void Game::spawnProjectiles(float mouseX, float mouseY, player* spawningPlayer, int _projNum)
{
	utils::mutex.lock();

	if(spawningPlayer->getArmStat().myWeaponType == armStat::gun)
	{
		projectile* proj;
		proj = new projectile(mouseX, mouseY, spawningPlayer);
		proj->projNum = _projNum;
	
		_gameObjectManager.Add(proj);

		allProjectiles.insert( std::pair<int,VisibleGameObject*>(proj->projNum,proj)  );
	}
	else if(spawningPlayer->getArmStat().myWeaponType == armStat::laser)
	{
		laser* newLaser;
		newLaser = new laser(mouseX, mouseY, spawningPlayer);
		newLaser->projNum = _projNum;

		_gameObjectManager.Add(newLaser);

		allProjectiles.insert( std::pair<int,VisibleGameObject*>(newLaser->projNum,newLaser)  );
	}
	else if(spawningPlayer->getArmStat().myWeaponType == armStat::cannon)
	{
		cannon* can;
		can = new cannon(mouseX, mouseY, spawningPlayer);
		can->projNum = _projNum;
	
		_gameObjectManager.Add(can);

		allProjectiles.insert( std::pair<int,VisibleGameObject*>(can->projNum,can)  );
	}
	else if(spawningPlayer->getArmStat().myWeaponType == armStat::grenadeLauncher)
	{
		grenade* nade;
		nade = new grenade(mouseX, mouseY, spawningPlayer);
		nade->projNum = _projNum;
	
		_gameObjectManager.Add(nade);

		allProjectiles.insert( std::pair<int,VisibleGameObject*>(nade->projNum,nade)  );
	}
	else if(spawningPlayer->getArmStat().myWeaponType == armStat::railGun)
	{
		railGun* rail;
		rail = new railGun(mouseX, mouseY, spawningPlayer);
		rail->projNum = _projNum;

		_gameObjectManager.Add(rail);

		allProjectiles.insert( std::pair<int,VisibleGameObject*>(rail->projNum,rail)  );
	}



	utils::mutex.unlock();

}

void Game::spawnMissiles(float mouseX, float mouseY, player* spawningPlayer, int _projNum)
{
	utils::mutex.lock();

	missile* _missile;
	_missile = new missile( mouseX, mouseY, spawningPlayer);
	_missile->projNum = _projNum;
	assert( _missile->projNum >= 0 );

	_gameObjectManager.Add(_missile);

	allProjectiles.insert( std::pair<int,VisibleGameObject*>(_missile->projNum,_missile)  );

	std::map<int,VisibleGameObject*>::iterator it = Game::allProjectiles.find(_projNum);
	assert(it->first == it->second->projNum);

	std::cout << "proj num: " << _projNum << std::endl;

	utils::mutex.unlock();

}

void Game::handleMultiplayerEvent(sf::Event & currentEvent)
{
	sf::Packet outPacket;

	networkEvent currentNetEvent;
	bool needToSend = false;

	currentNetEvent.playerNumber = multiplayer::myPlayerNum;

	if(currentEvent.type == sf::Event::LostFocus)
	{
		isFocused = false;

		currentNetEvent.myType = networkEvent::lostFocus;
		needToSend = true;
	}

	else if(currentEvent.type == sf::Event::GainedFocus)
		isFocused = true;

	else if(currentEvent.type == sf::Event::Closed)
	{
		quitMultiplayer();
		_gameState = Game::Exiting;
	}

	else if(currentEvent.type == sf::Event::KeyPressed && isFocused) 
	{
		currentNetEvent.myType = networkEvent::moveStart;
		needToSend = true;

		if(currentEvent.key.code == sf::Keyboard::W)
			currentNetEvent.myMoveKey = networkEvent::up;
		else if(currentEvent.key.code == sf::Keyboard::A)
			currentNetEvent.myMoveKey = networkEvent::left;
		else if(currentEvent.key.code == sf::Keyboard::S)
			currentNetEvent.myMoveKey = networkEvent::down;
		else if(currentEvent.key.code == sf::Keyboard::D)
			currentNetEvent.myMoveKey = networkEvent::right;

		// launch pause menu
		else if(currentEvent.key.code == sf::Keyboard::Escape)
		{
			if(isPaused)
			{
				isPaused = false;
				pauseMenu.setShowing(false);
			}
			else
			{
				isPaused = true;
				pauseMenu.setShowing(true);
			}

			needToSend = false;
		}

		else
			needToSend = false;
	}

	else if(currentEvent.type == sf::Event::KeyReleased && isFocused) 
	{
		currentNetEvent.myType = networkEvent::moveStop;
		needToSend = true;

		if(currentEvent.key.code == sf::Keyboard::W)
			currentNetEvent.myMoveKey = networkEvent::up;
		else if(currentEvent.key.code == sf::Keyboard::A)
			currentNetEvent.myMoveKey = networkEvent::left;
		else if(currentEvent.key.code == sf::Keyboard::S)
			currentNetEvent.myMoveKey = networkEvent::down;
		else if(currentEvent.key.code == sf::Keyboard::D)
			currentNetEvent.myMoveKey = networkEvent::right;
		else
			needToSend = false;
	}

	if(needToSend)
	{
		if(multiplayer::_peerState == multiplayer::client)
		{
			outPacket << currentNetEvent;
			sender.send(outPacket,multiplayer::serverAddr,multiplayer::sendPort);
			outPacket.clear();
		}
		else if(multiplayer::_peerState == multiplayer::server)
		{
			outPacket << currentNetEvent;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}
	}

	if(isPaused)
		desktop.HandleEvent(currentEvent);

}

void Game::quitMultiplayer()
{
	sf::Packet outPacket;

	networkEvent leaveEvent;

	if(multiplayer::_peerState == multiplayer::client)
	{
		leaveEvent.myType = networkEvent::clientLeave;

		outPacket << leaveEvent;
		sender.send(outPacket,multiplayer::serverAddr,multiplayer::sendPort);
		outPacket.clear();
	}
	else if(multiplayer::_peerState == multiplayer::server)
	{
		leaveEvent.myType = networkEvent::serverLeave;

		outPacket << leaveEvent;
		multiplayer::sendToClients(outPacket);
		outPacket.clear();
	}

	
}

void Game::handleShotsFired()
{
	sf::Packet outPacket;

	networkEvent currentNetEvent;

	// fire arm
	if( sf::Mouse::isButtonPressed(sf::Mouse::Left) && isFocused && !myPlayer->getIsRespawning() && !myPlayer->armColliding() )
	{
		float timeDelta = myPlayer->cooldownTimer.getElapsedTime().asSeconds();
		// gun
		if(  ( myPlayer->getArmStat().myWeaponType == armStat::gun ||
			   myPlayer->getArmStat().myWeaponType == armStat::grenadeLauncher || 
			   myPlayer->getArmStat().myWeaponType == armStat::railGun ||
			   myPlayer->getArmStat().myWeaponType == armStat::flameThrower )
		 && timeDelta > myPlayer->getArmStat().firePeriod   )
		{

			(*myPlayer).cooldownTimer.restart();
			sf::Vector2f woldPos;
			woldPos = _mainWindow.convertCoords( sf::Mouse::getPosition(_mainWindow) );
			

			currentNetEvent.myType = networkEvent::shotFired;
			currentNetEvent.mouseX = woldPos.x;
			currentNetEvent.mouseY = woldPos.y;
			currentNetEvent.playerNumber = multiplayer::myPlayerNum;
			

			if(multiplayer::_peerState == multiplayer::client)
			{
				outPacket << currentNetEvent;
				sender.send(outPacket,multiplayer::serverAddr,multiplayer::sendPort);
				outPacket.clear();
			}
			else if(multiplayer::_peerState == multiplayer::server)
			{
				currentNetEvent.projNum = projectileCount;
				spawnProjectiles(woldPos.x, woldPos.y, myPlayer, projectileCount);
				
				projectileCount++;

				outPacket << currentNetEvent;
				multiplayer::sendToClients(outPacket);
				outPacket.clear();
			}

		}
		// laser
		else if( myPlayer->getArmStat().myWeaponType == armStat::laser && !myPlayer->isLasering )
		{
			myPlayer->isLasering = true;

			sf::Vector2f woldPos;
			woldPos = _mainWindow.convertCoords( sf::Mouse::getPosition(_mainWindow) );
			

			currentNetEvent.myType = networkEvent::shotFired;
			currentNetEvent.mouseX = woldPos.x;
			currentNetEvent.mouseY = woldPos.y;
			currentNetEvent.playerNumber = multiplayer::myPlayerNum;
			

			if(multiplayer::_peerState == multiplayer::client)
			{
				outPacket << currentNetEvent;
				sender.send(outPacket,multiplayer::serverAddr,multiplayer::sendPort);
				outPacket.clear();
			}
			else if(multiplayer::_peerState == multiplayer::server)
			{
				currentNetEvent.projNum = projectileCount;
				spawnProjectiles(woldPos.x, woldPos.y, myPlayer, projectileCount);
				
				projectileCount++;

				outPacket << currentNetEvent;
				multiplayer::sendToClients(outPacket);
				outPacket.clear();
			}

		}
		// cannon
		else if( myPlayer->getArmStat().myWeaponType == armStat::cannon && !myPlayer->isCannonCharging )
		{
			currentNetEvent.myType = networkEvent::chargeStart;
			currentNetEvent.playerNumber = multiplayer::myPlayerNum;

			if(multiplayer::_peerState == multiplayer::client)
			{
				outPacket << currentNetEvent;
				sender.send(outPacket,multiplayer::serverAddr,multiplayer::sendPort);
				outPacket.clear();
			}
			else if(multiplayer::_peerState == multiplayer::server)
			{
				outPacket << currentNetEvent;
				multiplayer::sendToClients(outPacket);
				outPacket.clear();
			}

			myPlayer->isCannonCharging = true;
			myPlayer->chargeTimer.restart();

		}


	}

	// fire missiles
	if( sf::Mouse::isButtonPressed(sf::Mouse::Right) && isFocused && !myPlayer->getIsRespawning() )
	{
		int numMissiles;
		if (myPlayer->getShoulderStat().myWeaponType == shoulderStat::cluster)
			numMissiles = 3;
		else
			numMissiles = 1;

		//std::cout << "firing: " << numMissiles << std::endl;


		float timeDelta = myPlayer->missileTimer.getElapsedTime().asSeconds();
		if( timeDelta > 1.0f )
		{

			(*myPlayer).missileTimer.restart();


			for (int i=0; i<numMissiles; i++)
			{

				sf::Vector2f woldPos;
				woldPos = _mainWindow.convertCoords( sf::Mouse::getPosition(_mainWindow) );


				// separate the cluster
				sf::Vector2f normal(woldPos.y-myPlayer->GetPosition().y,myPlayer->GetPosition().x-woldPos.x);
				utils::normalize(normal);
				if (i == 1)
					woldPos+=normal*50.f;
				else if (i ==2)
					woldPos-=normal*50.f;

				currentNetEvent.myType = networkEvent::missileFired;
				currentNetEvent.mouseX = woldPos.x;
				currentNetEvent.mouseY = woldPos.y;
				currentNetEvent.playerNumber = multiplayer::myPlayerNum;

				if(multiplayer::_peerState == multiplayer::client)
				{
					outPacket << currentNetEvent;
					sender.send(outPacket,multiplayer::serverAddr,multiplayer::sendPort);
					outPacket.clear();
				}
				else if(multiplayer::_peerState == multiplayer::server)
				{
					currentNetEvent.projNum = projectileCount;
					spawnMissiles(woldPos.x, woldPos.y, myPlayer,projectileCount);

					projectileCount++;

					outPacket << currentNetEvent;
					multiplayer::sendToClients(outPacket);
					outPacket.clear();
				}
			}
		}
	}



	// fire cannon on release
	if( !sf::Mouse::isButtonPressed(sf::Mouse::Left) && isFocused && !myPlayer->getIsRespawning() && myPlayer->isCannonCharging && !myPlayer->armColliding() )
	{
		myPlayer->isCannonCharging = false;
		std::cout << myPlayer->chargeTimer.getElapsedTime().asSeconds() << std::endl;

		sf::Vector2f woldPos = _mainWindow.convertCoords( sf::Mouse::getPosition(_mainWindow) );

		currentNetEvent.myType = networkEvent::shotFired;
		currentNetEvent.mouseX = woldPos.x;
		currentNetEvent.mouseY = woldPos.y;
		currentNetEvent.playerNumber = multiplayer::myPlayerNum;
		

		if(multiplayer::_peerState == multiplayer::client)
		{
			outPacket << currentNetEvent;
			sender.send(outPacket,multiplayer::serverAddr,multiplayer::sendPort);
			outPacket.clear();
		}
		else if(multiplayer::_peerState == multiplayer::server)
		{
			currentNetEvent.projNum = projectileCount;
			spawnProjectiles(woldPos.x, woldPos.y, myPlayer, projectileCount);
			
			projectileCount++;

			outPacket << currentNetEvent;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}

	}



}

void Game::playMultiplayer()
{
	launchBotPathing();

	handleShotsFired();
	
	if(multiplayer::_peerState == multiplayer::server)
		multiplayer::updateBots();

	_mainWindow.clear(sf::Color(100,100,255));

	backgroundManager();

	_gameObjectManager.UpdateAll();
	sendArmRotation();

	_gameObjectManager.deleteFlagged();

	_mainWindow.setView( myPlayer->getView() );


	BoxManager();
	_gameObjectManager.DrawAll(_mainWindow);
	

	showHealth();
	showFuel();

	if(myPlayer->hasRadar)
		showRadar();

	// out for sp testign

	//if(multiplayer::_peerState == multiplayer::server)
	//	checkVictory();

	if(isPaused)
	{
		Game::desktop.Update( pauseMenu.clock.restart().asSeconds() );
		sfg::Renderer::Get().Display(_mainWindow);
	}

	_mainWindow.display();

	// if exiting to menu need to delete thread and free memory
	if(Game::_gameState == Game::ShowingMenu)
		Game::cleanUp();

}

void Game::sendArmRotation()
{
	float angle = myPlayer->getArmRotation();

	//std::cout << angle << std::endl;

	float cmpAng1;
	float cmpAng2;

	cmpAng1 = angle;
	cmpAng2 = prevAngle;

	if(cmpAng1 < 0)
		cmpAng1+=360;

	if(cmpAng2 < 0)
		cmpAng2+=360;

	//std::cout << cmpAng1 << std::endl;

	const float pi = 3.14159265f;
	if(  (cos(angle*pi/180) < 0 && cos(prevAngle*pi/180) > 0) || (cos(angle*pi/180) > 0 && cos(prevAngle*pi/180) < 0)  || std::abs(cmpAng2-cmpAng1) > 5.0f )
	{

		sf::Packet outPacket;

		networkEvent currentNetEvent;
		currentNetEvent.myType = networkEvent::armRotate;
		currentNetEvent.angle = myPlayer->getArmRotation();
		currentNetEvent.playerNumber = multiplayer::myPlayerNum;

		prevAngle = currentNetEvent.angle;

		if(multiplayer::_peerState == multiplayer::client)
		{
			outPacket << currentNetEvent;
			sender.send(outPacket,multiplayer::serverAddr,multiplayer::sendPort);
			outPacket.clear();
		}
		else if(multiplayer::_peerState == multiplayer::server)
		{
			outPacket << currentNetEvent;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}
	}
}

void Game::restartMenu()
{
	mainMenu.restart();
}

void Game::quitToDesktop()
{
	pauseMenu.setShowing(false);

	// clean up comes when game enter new state
	Game::_gameState = Exiting;
	Game::quitMultiplayer();
}

void Game::quitToMenu(bool isServerDisconnect)
{
	// clean up comes when game enter new state
	Game::_gameState = Game::ShowingMenu;
	Game::restartMenu();
	isPaused = false;
	pauseMenu.setShowing(false);

	if (!isServerDisconnect)
		Game::quitMultiplayer();
}


void Game::quitToLobby(bool isServerDisconnect)
{
	// clean up comes when game enter new state
	Game::_gameState = Game::ShowingMenu;
	Game::restartMenu();
	isPaused = false;
	pauseMenu.setShowing(false);

	isReHosting = true;

	multiplayer::copyPeers();

}


void Game::cleanUp()
{
	
	if(musicPlaying)
	{
		curMusic->stop();
		delete curMusic;
		musicPlaying = false;
	}


	mySoundManager.deleteAll();

	projectileCount = 0;

	std::cout << "about to end network trheads" << std::endl;
	endNetworkThreads();

	std::cout << "about to free peers" << std::endl;
	multiplayer::freePeers();//locking up in heres

	std::cout << "about to disable pause" << std::endl;
	Game::pauseMenu.setShowing(false);

	std::cout << "about to destroy map" << std::endl;
	if(mapIsInMemory)
		mapDestroyer();

	

	

	std::cout << "about to free lobby icons" << std::endl;
	multiplayer::freeLobyIcons();
	std::cout << "about to delete object" << std::endl;
	Game::_gameObjectManager.deleteAll();
	std::cout << "done cleaning" << std::endl;
}


void Game::endNetworkThreads()
{
	if ( multiplayer::_peerState == multiplayer::server)
	{
		network_server_listen.terminate();
		network_server_send_sync.terminate();
	}
	else if ( multiplayer::_peerState == multiplayer::client)
	{
		network_client_listen.terminate();
	}


	multiplayer::unbindSockets();
}

bool Game::readName()
{

	//std::cout << "saving" << std::endl;

	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return false;

	std::string saveDirectory = path;

	saveDirectory.append("\\Documents\\My Games\\RADgame\\profile\\");
	saveDirectory.append("name.txt");

	
	std::ifstream nameFile;
	nameFile.open(saveDirectory);

	if (nameFile.good())
	{
		nameFile >> myName;
		nameFile.close();
		return true;
	}
	else
	{
		myName = "mingeBag";
		nameFile.close();
		return false;
	}


}
	

bool Game::writeName(std::string _name)
{
	myName = _name;

	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return false;

	std::string saveDirectory = path;
	saveDirectory.append("\\Documents\\My Games\\RADgame\\profile\\");
	
	std::string command = "mkdir \"";
	command.append(saveDirectory);
	command.append("\"");

	if(!utils::directoryExists(saveDirectory))
		system(command.c_str());

	saveDirectory.append("name.txt");

	std::ofstream nameFile;

	nameFile.open(saveDirectory);
	nameFile << _name << std::endl;
	nameFile.close();

	return true;



}

void Game::showHealth()
{
	if(myPlayer->getIsDead())
	{
		sf::Text deathDisplay("You are dead");
		sf::Vector2f viewPos = myPlayer->getView().getCenter();
		sf::FloatRect textRect = deathDisplay.getLocalBounds();
		deathDisplay.setPosition(viewPos.x-textRect.width/2, viewPos.y - textRect.height/2);
		_mainWindow.draw(deathDisplay);
	}
	else
	{
		sf::Text healthDisplay;
		std::stringstream healthText;
		healthText << myPlayer->getHealth();
		healthDisplay.setString( healthText.str() );

		sf::Vector2f viewPos = myPlayer->getView().getCenter();
		sf::Vector2f viewSize = myPlayer->getView().getSize();

		sf::FloatRect textRect = healthDisplay.getLocalBounds();

		healthDisplay.setPosition(viewPos.x-viewSize.x/2+25.0f,viewPos.y+viewSize.y/2-textRect.height-25.0f);
		_mainWindow.draw(healthDisplay);


		sf::Text livesDisplay;
		std::stringstream livesText;
		livesText << myPlayer->getLives();
		livesDisplay.setString( livesText.str() );


		textRect = livesDisplay.getLocalBounds();
		sf::FloatRect healthRect = healthDisplay.getGlobalBounds();

		livesDisplay.setPosition(healthRect.left,healthRect.top-textRect.height-25.0f);
		_mainWindow.draw(livesDisplay);
	}
}

void Game::showFuel()
{
	sf::Text fuelDisp;
	std::stringstream fuelTxt;

	int roundedFuel = (int) floor(myPlayer->getFuel() * 10.0f);


	fuelTxt << roundedFuel;
	fuelDisp.setString( fuelTxt.str() );

	sf::Vector2f viewPos = myPlayer->getView().getCenter();
	sf::Vector2f viewSize = myPlayer->getView().getSize();

	sf::FloatRect textRect = fuelDisp.getLocalBounds();

	fuelDisp.setPosition(viewPos.x+viewSize.x/2-textRect.width-25.0f,viewPos.y+viewSize.y/2-textRect.height-25.0f);
	_mainWindow.draw(fuelDisp);
}

void Game::showRadar()
{
	sf::Vector2f viewPos = myPlayer->getView().getCenter();
	sf::Vector2f viewSize = myPlayer->getView().getSize();

	sf::FloatRect viewRect(viewPos.x-viewSize.x/2,viewPos.y-viewSize.y/2,viewSize.x,viewSize.y);

	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it != multiplayer::allPeers.end(); ++it)
	{
		bool isAlive = !it->second->hisPlayer->getIsDead() && !it->second->hisPlayer->getIsRespawning();
		bool isOnScreen = viewRect.contains(it->second->hisPlayer->getCenterMass());

		if( isAlive &&  !isOnScreen)
		{

			sf::Text nameDisp;
			nameDisp.setString(it->second->name);

			sf::Vector2f delta = it->second->hisPlayer->getCenterMass() - viewPos;
			sf::Vector2f pos = viewPos;

			utils::normalize(delta);

			while(viewRect.contains(pos))
			{
				pos+=delta;
			}
			

			VisibleGameObject arrow;
			arrow.loadFromTex(images::misc::arrow);
			sf::FloatRect arrRect = arrow.GetSprite().getLocalBounds();
			arrow.GetSprite().setOrigin(arrRect.left+arrRect.width,arrRect.top+arrRect.height/2);
			arrow.SetPosition(viewPos.x,viewPos.y);
			arrow.rotateToFace(pos.x,pos.y);
			arrow.SetPosition(pos.x,pos.y);

			arrow.GetSprite().setColor(it->second->hisPlayer->GetSprite().getColor());
			arrow.Draw(_mainWindow);

			/*
			pos-=delta*40.0f;

			sf::FloatRect textRect = nameDisp.getLocalBounds();
			if(pos.x + textRect.width > viewRect.left+viewRect.width)
				pos.x-=textRect.width;
			if(pos.y + textRect.height > viewRect.top+viewRect.height)
				pos.y-=textRect.height;

		
			nameDisp.setPosition(pos.x,pos.y);
			_mainWindow.draw(nameDisp);
			*/
			
		}
	}
}

void Game::checkVictory()
{
	int deadCount = 0;
	winningPlayerNum = 0;
	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it != multiplayer::allPeers.end(); ++it)
	{
		if(it->second->hisPlayer->getIsDead())
			deadCount++;
		else
			winningPlayerNum=it->first;
	}

	if ( deadCount+1 == multiplayer::allPeers.size() )
	{
		_gameState = GameOver;

		networkEvent gameOverEvent;
		sf::Packet outPacket;

		gameOverEvent.myType = networkEvent::gameOver;
		gameOverEvent.winnerNum = winningPlayerNum;

		outPacket << gameOverEvent;
		multiplayer::sendToClients(outPacket);
		outPacket.clear();

	}


}

void Game::showGameOverScreen(sf::Event currentEvent)
{
	if(currentEvent.type == sf::Event::LostFocus)
		isFocused = false;

	else if(currentEvent.type == sf::Event::GainedFocus)
		isFocused = true;

	else if(currentEvent.type == sf::Event::Closed)
	{
		quitMultiplayer();
		_gameState = Game::Exiting;
	}

	Game::pauseMenu.setShowing(true);

	
	_mainWindow.clear(sf::Color(100,100,255));


	_mainWindow.setView( myPlayer->getView() );


	std::string winText = "The winner is: ";
	winText.append(multiplayer::allPeers.find(Game::winningPlayerNum)->second->name);

	sf::Text winDisplay(winText);
	sf::Vector2f viewPos = myPlayer->getView().getCenter();
	sf::FloatRect textRect = winDisplay.getLocalBounds();
	winDisplay.setPosition(viewPos.x-textRect.width/2, viewPos.y - textRect.height/2);
	_mainWindow.draw(winDisplay);


	Game::desktop.Update( pauseMenu.clock.restart().asSeconds() );
	sfg::Renderer::Get().Display(_mainWindow);



	desktop.HandleEvent(currentEvent);
	_mainWindow.display();

	// if exiting to menu need to delete thread and free memory
	if(Game::_gameState == Game::ShowingMenu)
	{
		std::cout << "cleaning" << std::endl;
		Game::cleanUp();
	}
}

void Game::setWinner(int winnerNum)
{
	_gameState = GameOver;
	winningPlayerNum=winnerNum;
}

void Game::backgroundManager()
{
	sf::Vector2f viewPos = myPlayer->getView().getCenter();
	sf::Vector2f viewSize = myPlayer->getView().getSize();
	background.SetPosition(viewPos.x - viewSize.x/2,viewPos.y - viewSize.y/2);
	background.Draw(_mainWindow);

	int midFactor = 2;

	midground.GetSprite().setTextureRect(sf::IntRect((int)(viewPos.x - viewSize.x/2)/midFactor,(int)(viewPos.y - viewSize.y/2)/midFactor,Game::myConfig.resolution.x,Game::myConfig.resolution.y));
	midground.SetPosition(viewPos.x - viewSize.x/2,viewPos.y - viewSize.y/2);
	midground.Draw(_mainWindow);
}

void Game::handleBotShots(player* player, bool leftPressed, bool rightPressed, sf::Vector2f target)
{
	sf::Packet outPacket;

	networkEvent currentNetEvent;

	// fire arm
	if( leftPressed && !player->getIsRespawning() )
	{
		float timeDelta = player->cooldownTimer.getElapsedTime().asSeconds();
		// gun
		if(  ( player->getArmStat().myWeaponType == armStat::gun ||
			   player->getArmStat().myWeaponType == armStat::grenadeLauncher || 
			   player->getArmStat().myWeaponType == armStat::railGun ||
			   player->getArmStat().myWeaponType == armStat::flameThrower )
		 && timeDelta > player->getArmStat().firePeriod   )
		{

			(*player).cooldownTimer.restart();

			currentNetEvent.myType = networkEvent::shotFired;
			currentNetEvent.mouseX = target.x;
			currentNetEvent.mouseY = target.y;
			currentNetEvent.playerNumber = player->getPlayerNum();
		
			currentNetEvent.projNum = projectileCount;
			spawnProjectiles(target.x, target.y, player, projectileCount);
			
			projectileCount++;

			outPacket << currentNetEvent;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();

		}
		// laser
		else if( player->getArmStat().myWeaponType == armStat::laser && !player->isLasering )
		{
			player->isLasering = true;
			
			currentNetEvent.myType = networkEvent::shotFired;
			currentNetEvent.mouseX = target.x;
			currentNetEvent.mouseY = target.y;
			currentNetEvent.playerNumber = player->getPlayerNum();
			
			currentNetEvent.projNum = projectileCount;
			spawnProjectiles(target.x, target.y, player, projectileCount);
			
			projectileCount++;

			outPacket << currentNetEvent;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();

		}
		// cannon
		else if( player->getArmStat().myWeaponType == armStat::cannon && !player->isCannonCharging )
		{
			currentNetEvent.myType = networkEvent::chargeStart;
			currentNetEvent.playerNumber = player->getPlayerNum();

			outPacket << currentNetEvent;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();

			player->isCannonCharging = true;
			player->chargeTimer.restart();

		}


	}

	// fire missiles
	if( rightPressed && !player->getIsRespawning() )
	{
		int numMissiles;
		if (player->getShoulderStat().myWeaponType == shoulderStat::cluster)
			numMissiles = 3;
		else
			numMissiles = 1;

		//std::cout << "firing: " << numMissiles << std::endl;


		float timeDelta = player->missileTimer.getElapsedTime().asSeconds();
		if( timeDelta > 1.0f )
		{

			(*player).missileTimer.restart();


			for (int i=0; i<numMissiles; i++)
			{

				sf::Vector2f tempTar = target;

				// separate the cluster
				sf::Vector2f normal(tempTar.y-player->GetPosition().y,player->GetPosition().x-tempTar.x);
				utils::normalize(normal);
				if (i == 1)
					tempTar+=normal*50.f;
				else if (i ==2)
					tempTar-=normal*50.f;

				currentNetEvent.myType = networkEvent::missileFired;
				currentNetEvent.mouseX = tempTar.x;
				currentNetEvent.mouseY = tempTar.y;
				currentNetEvent.playerNumber = player->getPlayerNum();

				currentNetEvent.projNum = projectileCount;
				spawnMissiles(tempTar.x, tempTar.y, player,projectileCount);

				projectileCount++;

				outPacket << currentNetEvent;
				multiplayer::sendToClients(outPacket);
				outPacket.clear();
			}
		}
	}



	// fire cannon on release
	if( !leftPressed && !player->getIsRespawning() && player->isCannonCharging )
	{
		player->isCannonCharging = false;
		std::cout << player->chargeTimer.getElapsedTime().asSeconds() << std::endl;

		currentNetEvent.myType = networkEvent::shotFired;
		currentNetEvent.mouseX = target.x;
		currentNetEvent.mouseY = target.y;
		currentNetEvent.playerNumber = player->getPlayerNum();
		
		currentNetEvent.projNum = projectileCount;
		spawnProjectiles(target.x, target.y, player, projectileCount);
		
		projectileCount++;

		outPacket << currentNetEvent;
		multiplayer::sendToClients(outPacket);
		outPacket.clear();

	}


}

void Game::checkMapBounds(int & i, int & j)
{
	if (i < 0)
		i = 0;
	if (i >= Game::mapWidth)
		i = Game::mapWidth-1;

	if (j < 0)
		j = 0;
	if (j >= Game::mapHeight)
		j = Game::mapHeight-1;
}


void Game::launchBotPathing()
{
	
	float maxTime = 0;
	bool pathInProgress = false;
	int numToPathFor;
	bool botNeedsPath = false;

	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it != multiplayer::allPeers.end(); ++it)
	{
		if(it->second->isBot && !it->second->hisPlayer->getIsDead() )
		{

			if(it->second->pathTimer.getElapsedTime().asSeconds() > maxTime)
			{
				maxTime = it->second->pathTimer.getElapsedTime().asSeconds();
				numToPathFor = it->first;
				botNeedsPath = true;
			}

			if(it->second->pathingInProgress)
				pathInProgress = true;
		}
	}

	if(!pathInProgress && Game::needCleanceReCalc && Game::clearanceTimer.getElapsedTime().asSeconds() > 0.5f)
	{
		clearance_recalculator.launch();
		needCleanceReCalc = false;
	}
	else if(!pathInProgress && botNeedsPath && maxTime > 0.5f && !reCalcingClearance)
	{
		multiplayer::allPeers.find(numToPathFor)->second->path_finder->launch();
		multiplayer::allPeers.find(numToPathFor)->second->pathTimer.restart();
	}


}

void Game::reCalcClearance()
{
	reCalcingClearance = true;
	for(int i = 0; i<mapWidth; i++)
	{
		for(int j = 0; j<mapHeight; j++)
		{
			Game::gameGrid[i][j].calcClearance();
		}
	}
	reCalcingClearance = false;
}

bool Game::loadMapNames()
{
	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return false;

	std::string mapDir = path;
	mapDir.append("\\Documents\\My Games\\RADgame\\maps\\");

	DIR *dir;
	struct dirent *ent;
	dir = opendir (mapDir.c_str());
	size_t isMap;

	if (dir != NULL) 
	{

		while ((ent = readdir (dir)) != NULL) 
		{
			//printf ("%s\n", ent->d_name);
			std::string fileName = ent->d_name;
			isMap = fileName.find(".txt");
			if (isMap != std::string::npos)
			{
				// chop off .txt from button
				fileName.erase(fileName.length()-4,fileName.length());
				
				mapNames.insert(fileName);
			}

		}
		closedir (dir);
	} 
	else 
	{
		/* could not open directory */
		return false;
	}

	if(mapNames.size() > 0)
		Game::selectedMap = *mapNames.begin();

	return true;

}

sf::Thread Game::clearance_recalculator(&Game::reCalcClearance);
sf::Clock Game::clearanceTimer;

sf::Thread Game::network_server_listen(&multiplayer::packetListener);
sf::Thread Game::network_client_listen(&multiplayer::packetListener);
sf::Thread Game::network_server_send_sync(&multiplayer::serverUpdatePositions);

Game::GameState Game::_gameState = Uninitialized;
sf::RenderWindow Game::_mainWindow;
GameObjectManager Game::_gameObjectManager;
box** Game::gameGrid;

player* Game::myPlayer;

int Game::mapWidth;
int Game::mapHeight;

int Game::lives;

bool Game::isFocused;

mechSpec Game::activeSpec;
std::string Game::myName;

sfg::SFGUI Game::m_sfgui;
sfg::Desktop Game::desktop;

MainMenu Game::mainMenu;
buildMenu* Game::myBuildMenu;
pauseMenu Game::pauseMenu;
messageWindow Game::messageWindow;

bool Game::isPaused;
bool Game::mapIsInMemory;

int Game::winningPlayerNum;

bool Game::isReHosting;
bool Game::isReJoining;

int Game::projectileCount;
std::map<int,VisibleGameObject*> Game::allProjectiles;

VisibleGameObject Game::background;
VisibleGameObject Game::midground;

Game::gameType Game::myGameType;

bool Game::ffIsOn;

bool Game::needCleanceReCalc;
bool Game::reCalcingClearance;

config Game::myConfig;

soundManager Game::mySoundManager;

sf::Music* Game::curMusic;
bool Game::musicPlaying;

sf::UdpSocket Game::sender;

float Game::prevAngle;

std::string Game::selectedMap;

std::set<std::string> Game::mapNames;

std::map<std::string, sf::Texture> Game::mapTileTextures;