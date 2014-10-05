#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "SFML/Audio.hpp"
#include <SFML/Network.hpp>

#include "mechSpec.h"
#include "peer.h"

#include <string>
#include <queue>

class lobby;

class multiplayer
{
public:
	static sf::Packet latestPacket;

	static void setupMultiplayer();
	static void StartServer();
	static void StartClient(std::string ip);

	static void serverUpdatePositions();

	static void packetListener();

	static void processIncPacket(sf::Packet);
	static void processLobbyPacket(sf::Packet, sf::TcpSocket & client, int newPlayerNum);

	static sf::IpAddress serverAddr;

	static unsigned short sendPort;
	static unsigned short listenPort;
	static unsigned short tcpPort;

	enum PeerState {  client, server };
	static PeerState _peerState;

	static int myPlayerNum;
	static int playerCounter;

	static std::map<int,peer*> allPeers;
	static std::map<int,peer*> reJoinPeers;

	static void copyPeers();
	static void sendReJoin();
	static void destroyPeerCopy();

	static void sendToClients(sf::Packet packet);

	static void setGameStarted(bool _isStarted);
	static void setExitingLobby(bool _isExiting);

	static bool getGameStarted();

	static void freePeers();
	static void freeLobyIcons();

	static bool quittingFromLobby;

	static void unbindSockets();

	static void sendToOthersTCP(int receivedPlayerNum, sf::Packet packet);
	static void sendToClientsTCP(sf::Packet packet);

	// client lobby socket
	static sf::TcpSocket* clientLobbySock;

	static void addBot();
	static void updateBots();

	static void receiveFile(std::string serverPath, std::string clientPath);

	static void clientMapReady();

private:
	static bool gameStarted;
	static bool exitingLobby;


	static void showLobby();

	static void sendToOthers(int receivedPlayerNum, sf::Packet packet);
	

	// clients used during lobby
	static std::map<int,sf::TcpSocket*> clients;

	

	static lobby lobby;

	//used in other threads for constant listening
	static sf::UdpSocket listener;

};

