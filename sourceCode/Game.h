#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include <SFML/Network.hpp>
#include <SFGUI/SFGUI.hpp>

#include "GameObjectManager.h"
#include "box.h"
#include "player.h"
#include "multiplayer.h"
#include "mechSpec.h"
#include "MainMenu.h"
#include "buildMenu.h"
#include "pauseMenu.h"
#include "messageWindow.h"
#include "soundManager.h"

#include "config.h"

class Game
{
public:
	static bool isFocused;

	static box** gameGrid;

	static int mapWidth;
	static int mapHeight;

	static void Start();
	static sf::RenderWindow& GetWindow();
	static GameObjectManager& GetGameObjectManager();


	static config myConfig;

	const static int GRID_WIDTH = 16;
	const static int GRID_HEIGHT = 16;

	static int lives;

	static player* myPlayer;


	static void spawnProjectiles(float, float, player*, int _projNum);
	static void spawnMissiles(float mouseX, float mouseY, player* spawningPlayer, int _projNum);

	// use pointer to avoid construction at game start
	static mechSpec activeSpec;
	static std::string myName;

	// Create an SFGUI. This is required before doing anything with SFGUI.
    static sfg::SFGUI m_sfgui;
	static sfg::Desktop desktop;

	static void restartMenu();

	static void quitToDesktop();
	static void quitToMenu(bool isServerDisconnect);
	static void quitToLobby(bool isServerDisconnect);
	
	static bool readName();
	static bool writeName(std::string);

	static messageWindow messageWindow;

	static void checkVictory();

	static void setWinner(int winnerNum);

	static bool isReHosting;
	static bool isReJoining;

	static void cleanUp();

	static int projectileCount;
	static std::map<int,VisibleGameObject*> allProjectiles;

	enum gameType {ffa,teams};
	static gameType myGameType;

	static bool ffIsOn;

	static void handleBotShots(player* player, bool leftPressed, bool rightPressed, sf::Vector2f target);

	static void checkMapBounds(int & i, int & j);

	static bool needCleanceReCalc;
	static sf::Thread clearance_recalculator;
	static bool reCalcingClearance;

	static soundManager mySoundManager;

	static sf::Music* curMusic;
	static bool musicPlaying;

	static sf::UdpSocket sender;
	static float prevAngle;

	static std::string selectedMap;

	static std::set<std::string> mapNames;
	static bool loadMapNames();

	static std::map<std::string, sf::Texture> mapTileTextures;

	static void insertTileTexture( std::map<std::string, std::string> &renamedTextures, std::string &textureName, std::string &textureNameChecking, int &copyNumber, sf::Texture &tempTexture );

private:
	static void launchBotPathing();

	static void backgroundManager();
	static VisibleGameObject background;
	static VisibleGameObject midground;

	static int winningPlayerNum;

	static void showHealth();
	static void showFuel();
	static void showRadar();

	static void showGameOverScreen(sf::Event);

	static bool mapIsInMemory;

	

	static void endNetworkThreads();

	static bool isPaused;

	static void quitMultiplayer();

	static pauseMenu pauseMenu;
	static MainMenu mainMenu;

	// use a pointer since it requires images to initialize first
	static buildMenu* myBuildMenu;
	
	static void mapLoader(std::string mapName);
	static void mapDestroyer();

	static sf::RenderWindow _mainWindow;

	static bool IsExiting();
	static void GameLoop();
	
	static void ShowSplashScreen();
	static void ShowMenu();

	enum GameState { Uninitialized, ShowingSplash, Paused, 
					ShowingMenu, Playing, mechBuilding, Exiting, GameOver };

	static GameState _gameState;
	
	static GameObjectManager _gameObjectManager;

	static void BoxManager();

	static void playMultiplayer();
	static void handleMultiplayerEvent(sf::Event &);
	static void handleShotsFired();

	static void sendArmRotation();

	static void setupMultiplayer();

	static sf::Thread Game::network_server_listen;
	static sf::Thread Game::network_client_listen;
	static sf::Thread Game::network_server_send_sync;

	static void reCalcClearance();
	static sf::Clock clearanceTimer;

};

