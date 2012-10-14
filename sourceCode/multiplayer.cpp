#include "multiplayer.h"
#include "Game.h"
#include "packet.h"
#include "projectile.h"
#include "lobby.h"
#include "lobbyEvent.h"
#include "peer.h"
#include "utils.h"
#include "SHATools.h"

#include <SFML/Network.hpp>

#include <windows.h>
#include <shlobj.h>
#include <dirent.h>

void multiplayer::StartServer()
{
	playerCounter = 0;

	_peerState = server;
	myPlayerNum = playerCounter;
	peer* addingPeer = new peer(Game::myName, Game::activeSpec,sf::IpAddress::getLocalAddress());
	allPeers.insert( std::pair<int,peer*>(playerCounter,addingPeer) );
	playerCounter++;

	// add server icon to lobby
	lobby.addIcon(myPlayerNum,Game::myName);

	// Used later in game for UDP communication
	listenPort = 5555;
	sendPort = 7777;
	tcpPort = 3333;

	/*
	sf::IpAddress AddressLoc = sf::IpAddress::getLocalAddress();
	sf::IpAddress AddressPub = sf::IpAddress::getPublicAddress();
	std::string IP = AddressPub.toString();
	std::cout << "public address: " << IP << std::endl;
	IP = AddressLoc.toString();
	std::cout << "local address: " << IP << std::endl;
	*/

	// Create a socket to listen to new connections
	sf::TcpListener listener;
	listener.listen(tcpPort);

	

	// Create a selector
	sf::SocketSelector selector;

	// Add the listener to the selector
	selector.add(listener);

	// Endless loop that waits for new connections
	gameStarted = false;
	exitingLobby = false;
	quittingFromLobby = false;

	if (Game::isReHosting)
	{
		Game::isReHosting = false;
		multiplayer::sendReJoin();
		multiplayer::destroyPeerCopy();
	}

	multiplayer::allPeers.find(multiplayer::myPlayerNum)->second->mapReady = true;
	bool clientsReady = false;

	while (!(gameStarted && clientsReady) && !exitingLobby && !quittingFromLobby)
	{
		if(!gameStarted)
			lobby.Show(Game::GetWindow());
		
		clientsReady = true;
		for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it != multiplayer::allPeers.end(); ++it)
		{
			clientsReady = clientsReady && it->second->mapReady;
		}


		//if(clientsReady)
		//	std::cout << "ready" << std::endl;
		//else
		//	std::cout << "not ready" << std::endl;
		
		// Make the selector wait for data on any socket
		sf::Time timeOut = sf::microseconds(1);
		if (selector.wait(timeOut))
		{
			// Test the listener
			if (selector.isReady(listener))
			{
				// The listener is ready: there is a pending connection
				sf::TcpSocket* client = new sf::TcpSocket;
				if (listener.accept(*client) == sf::Socket::Done)
				{
					// Add the new client to the clients list
					clients.insert(std::pair<int,sf::TcpSocket*>(playerCounter,client));
					playerCounter++;
			

					// Add the new client to the selector so that we will
					// be notified when he sends something
					selector.add(*client);

					std::cout << "new connection from: " << client->getRemoteAddress() << std::endl;
				}
			}
			else
			{
				// The listener socket is not ready, test all other sockets (the clients)
				for (std::map<int, sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
				{
					sf::TcpSocket& client = *(it->second);
					if (selector.isReady(client))
					{
						// The client has sent some data, we can receive it
						sf::Packet packet;
						sf::Socket::Status result;
						result = client.receive(packet);
						if (result == sf::Socket::Done)
						{

							multiplayer::processLobbyPacket(packet,client, it->first);
							packet.clear();	
						}
						else if (result == sf::Socket::Disconnected)
						{
							std::cout << "disco" << std::endl;
							selector.remove(client);
							lobby.removeIcon(it->first);
							delete allPeers.find(it->first)->second;
							allPeers.erase(it->first);

							sf::Packet outPacket;
							lobbyEvent leaveEvent;
							leaveEvent.myType = lobbyEvent::playerLeave;
							leaveEvent.playerNum = it->first;
							outPacket << leaveEvent;
							sendToOthersTCP(it->first,outPacket);
						}
					}
				}
			}
		}
		


	}


	//std::cout << "exit while" << std::endl;
	//std::cout << gameStarted << exitingLobby << quittingFromLobby << clientsReady << std::endl;

	// start the game for the clients
	if(gameStarted)
	{
		lobbyEvent startEvent;
		startEvent.myType = lobbyEvent::gameStart;
		sf::Packet outPacket;
		outPacket << startEvent;
		sendToClientsTCP(outPacket);
		outPacket.clear();
	}

	for (std::map<int, sf::TcpSocket*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		it->second->disconnect();
		delete it->second;
	}
	clients.clear();
}

void multiplayer::StartClient(std::string ip)
{
	clientLobbySock = new sf::TcpSocket;

	clientLobbySock->disconnect();

	std::cout<<"client start"<<std::endl;

	_peerState = client;

	// Used later in game for UDP communication
	sendPort = 5555;
	listenPort = 7777;
	tcpPort = 3333;

	sf::Socket::Status connectResult;
	sf::Time timeOut = sf::seconds(5.0f);
	connectResult = clientLobbySock->connect(ip, tcpPort,timeOut);

	sf::Packet outPacket;

	lobbyEvent connectEvent;
	connectEvent.myType = lobbyEvent::joinRequest;
	outPacket << connectEvent;
	clientLobbySock->send(outPacket);
	outPacket.clear();

	//outPacket << Game::activeSpec << Game::myName;
	//clientLobbySock->send(outPacket);

	serverAddr=clientLobbySock->getRemoteAddress();

	
	gameStarted = false;
	exitingLobby = false;
	quittingFromLobby = false;

	clientLobbySock->setBlocking(true);
	sf::Packet firstPack;
	clientLobbySock->receive(firstPack);
	sf::TcpSocket dummy1;
	processLobbyPacket(firstPack,dummy1,0);

	clientLobbySock->setBlocking(false);
	while(!gameStarted && !exitingLobby && !quittingFromLobby)
	{
		lobby.Show(Game::GetWindow());

		sf::Packet inPacket;
		sf::Socket::Status result;
		result = clientLobbySock->receive(inPacket);

		if (result == sf::Socket::Done)
		{
			sf::TcpSocket dummy;
			processLobbyPacket(inPacket,dummy,0);
			inPacket.clear();
		}
		else if (result == sf::Socket::Disconnected)
		{
			std::cout << "disco" << std::endl;
			lobby.leaveLobby();

			std::string messageText = "The host disconnected";
			Game::messageWindow.displayMessage(messageText);
		}

	}

	clientLobbySock->disconnect();
	delete clientLobbySock;
}


void multiplayer::packetListener()
{
	listener.bind(listenPort);

	unsigned short incPort;
	sf::IpAddress incAddress;
	sf::Packet inPacket;

	while (true)
	{
		listener.receive(inPacket,incAddress,incPort);
		processIncPacket(inPacket);
	}
}

void multiplayer::processIncPacket(sf::Packet Packet)
{
	int receivedtype;
	int receivedmoveKey;
	int receivedPlayerNum;

	sf::Packet PacketCopy;
	PacketCopy = Packet;

	Packet>>receivedtype;

	// move start
	if(receivedtype == networkEvent::moveStart)
	{

		Packet>>receivedmoveKey;
		Packet>>receivedPlayerNum;

		if(_peerState == server)
			sendToOthers(receivedPlayerNum,PacketCopy);

		if(receivedmoveKey == networkEvent::up )
		{
			allPeers.find(receivedPlayerNum)->second->upIsPressed = true;
		}
		if(receivedmoveKey == networkEvent::down)
		{
			allPeers.find(receivedPlayerNum)->second->downIsPressed = true;
		}
		if(receivedmoveKey == networkEvent::left)
		{
			allPeers.find(receivedPlayerNum)->second->leftIsPressed = true;
		}
		if(receivedmoveKey == networkEvent::right)
		{
			allPeers.find(receivedPlayerNum)->second->rightIsPressed = true;
		}
	}

	// move stop
	if(receivedtype == networkEvent::moveStop)
	{

		Packet>>receivedmoveKey;
		Packet>>receivedPlayerNum;

		if(_peerState == server)
			sendToOthers(receivedPlayerNum,PacketCopy);

		if(receivedmoveKey == networkEvent::up)
		{
			allPeers.find(receivedPlayerNum)->second->upIsPressed = false;
		}
		if(receivedmoveKey == networkEvent::down)
		{
			allPeers.find(receivedPlayerNum)->second->downIsPressed = false;
		}
		if(receivedmoveKey == networkEvent::left)
		{
			allPeers.find(receivedPlayerNum)->second->leftIsPressed = false;
		}
		if(receivedmoveKey == networkEvent::right)
		{
			allPeers.find(receivedPlayerNum)->second->rightIsPressed = false;
		}
	}

	if(receivedtype == networkEvent::shotFired)
	{
		float recMouseX, recMouseY;
		int receivedPlayerNum;
		int receivedProjNum;


		Packet>>recMouseX>>recMouseY>>receivedPlayerNum>>receivedProjNum;

		if(_peerState == server)
		{
			networkEvent currentNetEvent;

			currentNetEvent.myType = networkEvent::shotFired;
			currentNetEvent.playerNumber= receivedPlayerNum;
			currentNetEvent.mouseX = recMouseX;
			currentNetEvent.mouseY = recMouseY;
			currentNetEvent.projNum = Game::projectileCount;

			sf::Packet shotPacket;

			shotPacket << currentNetEvent;

			sendToClients(shotPacket);

			Game::spawnProjectiles(recMouseX, recMouseY, allPeers.find(receivedPlayerNum)->second->hisPlayer,Game::projectileCount );

			Game::projectileCount++;
		}
		else if(_peerState == client)
		{
			Game::spawnProjectiles(recMouseX, recMouseY, allPeers.find(receivedPlayerNum)->second->hisPlayer, receivedProjNum);
		}

	}

	if(receivedtype == networkEvent::missileFired)
	{
		float recMouseX, recMouseY;
		int receivedPlayerNum;
		int receivedProjNum;


		Packet>>recMouseX>>recMouseY>>receivedPlayerNum>>receivedProjNum;

		if(_peerState == server)
		{
			networkEvent currentNetEvent;

			currentNetEvent.myType = networkEvent::missileFired;
			currentNetEvent.playerNumber= receivedPlayerNum;
			currentNetEvent.mouseX = recMouseX;
			currentNetEvent.mouseY = recMouseY;
			currentNetEvent.projNum = Game::projectileCount;

			sf::Packet shotPacket;

			shotPacket << currentNetEvent;

			sendToClients(shotPacket);

			Game::spawnMissiles(recMouseX, recMouseY, allPeers.find(receivedPlayerNum)->second->hisPlayer,Game::projectileCount );

			Game::projectileCount++;
		}
		else if(_peerState == client)
		{
			Game::spawnMissiles(recMouseX, recMouseY, allPeers.find(receivedPlayerNum)->second->hisPlayer, receivedProjNum);
		}

		
	}

	if(receivedtype == networkEvent::boxHit)
	{
		int i, j, boxDamage;
		Packet>>i>>j>>boxDamage;

		Game::gameGrid[i][j].takeDamage(boxDamage);
	}

	if(receivedtype == networkEvent::playerHit)
	{
		int receivedPlayerNum, playerDamage;
		Packet>>receivedPlayerNum>>playerDamage;

		allPeers.find(receivedPlayerNum)->second->hisPlayer->takeDamage(playerDamage);
	}

	if(receivedtype == networkEvent::sync)
	{
		int playerNumToSync;
		float playerX;
		float playerY;
		float velX;
		float velY;
		Packet>>playerNumToSync>>playerX>>playerY>>velX>>velY;

		allPeers.find(playerNumToSync)->second->hisPlayer->syncPosition(playerX, playerY);
		allPeers.find(playerNumToSync)->second->hisPlayer->setVel(velX,velY);
	}

	if(receivedtype == networkEvent::lostFocus)
	{
		int receivedPlayerNum;
		Packet>>receivedPlayerNum;

		if(_peerState == server)
			sendToOthers(receivedPlayerNum,PacketCopy);

		allPeers.find(receivedPlayerNum)->second->upIsPressed = false;
		allPeers.find(receivedPlayerNum)->second->downIsPressed = false;
		allPeers.find(receivedPlayerNum)->second->leftIsPressed = false;
		allPeers.find(receivedPlayerNum)->second->rightIsPressed = false;
	}

	if(receivedtype == networkEvent::armRotate)
	{
		float recAngle;
		int receivedPlayerNum;

		Packet>>recAngle>>receivedPlayerNum;

		if(_peerState == server)
			sendToOthers(receivedPlayerNum,PacketCopy);

		allPeers.find(receivedPlayerNum)->second->hisPlayer->setArmRotation(recAngle);
	}

	if(receivedtype == networkEvent::clientLeave)
	{
		int receivedPlayerNum;
		Packet>>receivedPlayerNum;

		if(_peerState == server)
			sendToOthers(receivedPlayerNum,PacketCopy);

		peer* leavingPeer = allPeers.find(receivedPlayerNum)->second;
		leavingPeer->hisPlayer->kill();
		std::cout << leavingPeer->name << " has left the game" << std::endl;
	}

	if(receivedtype == networkEvent::serverLeave)
	{
		Game::quitToMenu(true);

		std::string messageText = "The host disconnected";
		Game::messageWindow.displayMessage(messageText);
	}

	if(receivedtype == networkEvent::gameOver)
	{
		int winnerNum;
		Packet>>winnerNum;

		Game::setWinner(winnerNum);
	}

	if(receivedtype == networkEvent::reJoin)
	{
		
		std::cout << "rejoin" << std::endl;
		Game::isReJoining = true;
		Game::quitToMenu(true);
	}

	if(receivedtype == networkEvent::destroyProjectile)
	{
		// this needs to be mutexed so that the iterator doesn't get invalidated between
		// find and destroy
		utils::mutex.lock();

		int projNum;
		int receivedPlayerNum;
		Packet>>projNum>>receivedPlayerNum;

		if(_peerState == server)
			sendToOthers(receivedPlayerNum,PacketCopy);


		std::map<int,VisibleGameObject*>::iterator it;

		it = Game::allProjectiles.find(projNum);

		if( it != Game::allProjectiles.end() )
			it->second->destroyProjectile();

		utils::mutex.unlock();
	}

	if(receivedtype == networkEvent::knockBack)
	{
		int receivedPlayerNum;
		sf::Vector2f knockDirection;
		float knockSecs;

		Packet>>receivedPlayerNum>>knockDirection.x>>knockDirection.y>>knockSecs;

		sf::Time knockTime = sf::seconds(knockSecs);

		allPeers.find(receivedPlayerNum)->second->hisPlayer->knockBack(knockDirection,knockTime);
	}
	if(receivedtype == networkEvent::chargeStart)
	{
		int receivedPlayerNum;
		Packet>>receivedPlayerNum;

		if(_peerState == server)
			sendToOthers(receivedPlayerNum,PacketCopy);

		allPeers.find(receivedPlayerNum)->second->hisPlayer->isCannonCharging = true;
		allPeers.find(receivedPlayerNum)->second->hisPlayer->chargeTimer.restart();
	}

}

void multiplayer::processLobbyPacket(sf::Packet Packet, sf::TcpSocket & client, int newPlayerNum)
{
	//std::cout << "received something" << std::endl;
	int receivedtype;

	sf::Packet packetCopy;
	packetCopy = Packet;

	Packet>>receivedtype;


	if(receivedtype == lobbyEvent::joinRequest)
	{
		// new player
		std::cout << "new player" << std::endl;

		mechSpec incSpec;
		std::string incName;
		Packet >> incSpec >> incName;

		peer* addingPeer = new peer(incName, incSpec, client.getRemoteAddress() );
		allPeers.insert( std::pair<int,peer*>(newPlayerNum,addingPeer) );

		// add client icon to lobby
		lobby.addIcon(newPlayerNum,incName);

		lobbyEvent connectEvent;
		connectEvent.myType = lobbyEvent::newConnection;
		connectEvent.playerNum = newPlayerNum;
		sf::Packet outPacket;
		outPacket << connectEvent;
		client.send(outPacket);
		outPacket.clear();

		lobbyEvent joinEvent;
		joinEvent.myType = lobbyEvent::playerJoin;
		joinEvent.playerNum = newPlayerNum;
		outPacket << joinEvent;
		sendToOthersTCP(newPlayerNum,outPacket);
		outPacket.clear();

	}

	else if(receivedtype == lobbyEvent::newConnection)
	{
		

		Packet>>multiplayer::myPlayerNum;
		Packet>>Game::lives;

		int typeNum;
		Packet >> typeNum;
		Game::myGameType = (Game::gameType) typeNum; 

		Packet>>Game::ffIsOn;

		Packet>>Game::selectedMap;

		int numberOfPeers;
		Packet>>numberOfPeers;

		std::cout << "connected to party of: " << numberOfPeers << std::endl;

		for (int i = 0; i < numberOfPeers; i++)
		{
			int incPlayerNum;
			std::string incName;
			mechSpec incSpec;
			int teamNum;

			Packet >> incPlayerNum >> incName >> incSpec >> teamNum;
			peer* addingPeer = new peer( incName, incSpec );
			addingPeer->team = (peer::_team) teamNum;
			allPeers.insert( std::pair<int,peer*>(incPlayerNum,addingPeer) );
			lobby.addIcon(incPlayerNum,incName);
		}

	
	}

	else if(receivedtype == lobbyEvent::playerJoin)
	{
		std::cout << "other player joined" << std::endl;

		int incPlayerNum;
		std::string incName;
		mechSpec incSpec;

		Packet >> incPlayerNum >> incName >> incSpec;
		peer* addingPeer = new peer( incName, incSpec );
		allPeers.insert( std::pair<int,peer*>(incPlayerNum,addingPeer) );
		lobby.addIcon(incPlayerNum,incName);
	}

	else if(receivedtype == lobbyEvent::gameStart)
	{
		setGameStarted(true);
		lobby.freeIcons();
	}

	else if(receivedtype == lobbyEvent::playerLeave)
	{
		int incPlayerNum;
		Packet >> incPlayerNum;

		lobby.removeIcon(incPlayerNum);
		delete allPeers.find(incPlayerNum)->second;
		allPeers.erase(incPlayerNum);
	}

	else if(receivedtype == lobbyEvent::changeLives)
	{
		Packet >> Game::lives;
	}

	else if(receivedtype == lobbyEvent::changeSpec)
	{

		int incPlayerNum;
		mechSpec incSpec;

		Packet >> incPlayerNum >> incSpec;
		allPeers.find(incPlayerNum )->second->spec = incSpec;

		sendToOthersTCP(incPlayerNum,packetCopy);
	}

	else if(receivedtype == lobbyEvent::changeGameType)
	{
		int typeNum;
		Packet >> typeNum;
		Game::myGameType = (Game::gameType) typeNum;

		if(Game::myGameType == Game::teams)
		{
			for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it != multiplayer::allPeers.end(); ++it)
			{
				it->second->team = peer::red;
			}
		}
		else if(Game::myGameType == Game::ffa)
		{
			for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it != multiplayer::allPeers.end(); ++it)
			{
				it->second->team = peer::ffa;
			}
		}

	}

	else if(receivedtype == lobbyEvent::changeTeam)
	{

		int incPlayerNum;
		int teamNum;

		Packet >> incPlayerNum >> teamNum;

		allPeers.find(incPlayerNum )->second->team = (peer::_team) teamNum;

		sendToOthersTCP(incPlayerNum,packetCopy);
	}
	else if(receivedtype == lobbyEvent::changeFF)
	{
		Packet >> Game::ffIsOn;
	}
	else if(receivedtype == lobbyEvent::loadedMap)
	{
		int incPlayerNum;
		bool isReady;

		Packet >> incPlayerNum >> isReady;

		multiplayer::allPeers.find(incPlayerNum)->second->mapReady = isReady;

	}
	else if(receivedtype == lobbyEvent::changeMap)
	{
		Packet >> Game::selectedMap;
	}
	else if(receivedtype == lobbyEvent::launchMap)
	{
		std::string incHash;

		Packet >> incHash;

		std::cout << "map name: " << Game::selectedMap << std::endl;
		std::cout << "inc hash: " << incHash << std::endl;

		char homePath[ MAX_PATH ];
		SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, homePath );

		std::string mapDir = homePath;
		mapDir.append("\\Documents\\My Games\\RADgame\\maps\\");
		
		if( Game::mapNames.find(Game::selectedMap) == Game::mapNames.end() )
		{
			std::cout << "not found" << std::endl;
			//map not found by client need to transfer it over

			receiveFile("maps//" + Game::selectedMap + ".txt","maps//" + Game::selectedMap + ".txt");
			
			std::string fullPath = mapDir + Game::selectedMap + ".txt";
			
			std::string myHash;
			myHash = SHATools::getSha512Hash(fullPath, true); 
			//std::cout << "my hash: " << myHash << std::endl;

			if(myHash==incHash)
			{
				std::cout << "hashes now match" << std::endl;
				//hashes match, okay to select map

				clientMapReady();
			}
			else
			{
				std::cout << "hashes do not match, xfer fucked up" << std::endl;
			}

			
		}
		else
		{
			std::cout << "map found" << std::endl;
			//map was found, need to check hash

			std::string fullPath = mapDir + Game::selectedMap + ".txt";

			std::string myHash;
			myHash = SHATools::getSha512Hash(fullPath, true); 
			//std::cout << "my hash: " << myHash << std::endl;

			if(myHash==incHash)
			{
				std::cout << "hashes match" << std::endl;
				//hashes match, okay to select map

				clientMapReady();
			}
			else
			{
				std::cout << "hashes do not match" << std::endl;
				//hashes do not match
				//need to check copies
				//if no copies match send a file and append latest copies number


				for(int i=1; i<999; i++)
				{
					std::cout << "checking: " << Game::selectedMap + "_copy_" + utils::toString(i) << std::endl;

					std::string fullPath = mapDir + Game::selectedMap + "_copy_" + utils::toString(i) + ".txt";

					if( Game::mapNames.find( Game::selectedMap + "_copy_" + utils::toString(i) ) == Game::mapNames.end() )
					{
						// reached end of copies
						std::cout << "no matches found in copies" << std::endl;
						//need to transfer file andd append copy i

						receiveFile("maps//" + Game::selectedMap + ".txt","maps//" + Game::selectedMap  + "_copy_" + utils::toString(i) + ".txt");
						Game::selectedMap.append("_copy_" + utils::toString(i));
						std::cout << "now selecting: "  << Game::selectedMap << std::endl;

						std::string fullPath = mapDir + Game::selectedMap + ".txt";
						std::string myHash;
						myHash = SHATools::getSha512Hash(fullPath, true); 
						//std::cout << "my hash: " << myHash << std::endl;

						if(myHash==incHash)
						{
							std::cout << "hashes now match" << std::endl;
							//hashes match, okay to select map
							clientMapReady();
						}
						else
						{
							std::cout << "hashes do not match, xfer fucked up" << std::endl;
						}

						break;
					}
					else
					{
						//check another copy hash
						std::string copyHash;
						copyHash = SHATools::getSha512Hash(fullPath, true);

						if(copyHash==incHash)
						{
							Game::selectedMap.append("_copy_" + utils::toString(i));
							std::cout << "now selecting: "  << Game::selectedMap << std::endl;

							clientMapReady();
							break;
						}

					}

				}




			}


		}



	}
	else if(receivedtype == lobbyEvent::startTranfer)
	{
		std::string pathFromHome;
		Packet >> pathFromHome;

		char path[ MAX_PATH ];
		SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path );

		std::string fileDir = path;
		fileDir.append(pathFromHome);

		std::cout << "starting to transfer: " << fileDir << std::endl;

		std::ifstream inFile;
		inFile.open(fileDir);

		char Buffer[4096];

		while (inFile.good()) 
		{ 
			inFile.read(Buffer, sizeof(Buffer));
			client.send(Buffer, (std::size_t)inFile.gcount()*sizeof(char));

			std::cout << "read bytes: " << (std::size_t)inFile.gcount()*sizeof(char) << std::endl;
			std::cout << inFile.fail() << inFile.eof() << inFile.bad() << std::endl;
		}
		inFile.close();

		// indicates file is done
		Buffer[0] = 0;
		client.send(Buffer,sizeof(char));

	}

}

void multiplayer::clientMapReady()
{
	sf::Packet outPacket;
	lobbyEvent mapEv;

	mapEv.myType = lobbyEvent::loadedMap;
	mapEv.playerNum = multiplayer::myPlayerNum;
	mapEv.mapReady = true;

	outPacket << mapEv;
	clientLobbySock->send(outPacket);
	outPacket.clear();
}

// path starts in RADgame folder
void multiplayer::receiveFile(std::string serverPath, std::string clientPath)
{
	char path[ MAX_PATH ];
	SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path );

	std::string clientDir = path;
	clientDir.append("\\Documents\\My Games\\RADgame\\");
	clientDir.append(clientPath);

	std::ofstream outputFile;
	outputFile.open(clientDir);

	std::string serverDir = "\\Documents\\My Games\\RADgame\\";
	serverDir.append(serverPath);

	std::cout << "reqesting transfer: " << serverDir << std::endl;

	sf::Packet outPacket;
	lobbyEvent mapEv;

	mapEv.myType = lobbyEvent::startTranfer;
	mapEv.transferFilePath = serverDir;

	outPacket << mapEv;
	clientLobbySock->send(outPacket);
	outPacket.clear();

	clientLobbySock->setBlocking(true);
	while(true) 
	{
		char Buffer[4096];
		std::size_t Size = 0;

		clientLobbySock->receive(Buffer, sizeof(Buffer), Size); 
		std::cout << "read bytes: " << Size << std::endl;

		if( Buffer[Size-1] == 0 )
		{
			outputFile.write(Buffer, Size-1);

			std::cout << "done xfer" << std::endl;
			outputFile.close();
			break;
		}
		else
			outputFile.write(Buffer, Size);
	
	}
	clientLobbySock->setBlocking(false);
}

void multiplayer::sendToOthers(int receivedPlayerNum, sf::Packet packet)
{
	for(std::map<int,peer*>::iterator it = allPeers.begin(); it != allPeers.end(); ++it)
	{
		if ( it->first != receivedPlayerNum && it->first != myPlayerNum )
		{
			Game::sender.send(packet, it->second->address, multiplayer::sendPort);
		}
	}

}

void multiplayer::sendToClientsTCP( sf::Packet packet)
{

	for(std::map<int,sf::TcpSocket*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if ( it->first != myPlayerNum )
		{
			it->second->send(packet);
		}
	}
}

void multiplayer::sendToOthersTCP(int receivedPlayerNum, sf::Packet packet)
{

	for(std::map<int,sf::TcpSocket*>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if ( it->first != receivedPlayerNum && it->first != myPlayerNum )
		{
			it->second->send(packet);
		}
	}
}

void multiplayer::serverUpdatePositions()
{
	sf::Packet outPacket;
	networkEvent currentNetEvent;
	currentNetEvent.myType = networkEvent::sync;

	while (true)
	{
		for(std::map<int,peer*>::iterator it = allPeers.begin(); it != allPeers.end(); ++it)
		{
			currentNetEvent.playerNumToSync = it->first;
			currentNetEvent.playerX = it->second->hisPlayer->GetPosition().x;
			currentNetEvent.playerY = it->second->hisPlayer->GetPosition().y;
			currentNetEvent.velX = it->second->hisPlayer->getVel().x;
			currentNetEvent.velY = it->second->hisPlayer->getVel().y;
			outPacket.clear();
			outPacket << currentNetEvent;

			sendToClients(outPacket);
			outPacket.clear();
		}

		sf::Time waitTime = sf::seconds(1.0f);
		sf::sleep(waitTime);
	}
}

void multiplayer::sendToClients(sf::Packet packet)
{
	for(std::map<int,peer*>::iterator it = allPeers.begin(); it != allPeers.end(); ++it)
	{
		if ( it->first != myPlayerNum )
		{
			Game::sender.send(packet,it->second->address,multiplayer::sendPort);
		}
	}
}

void multiplayer::showLobby()
{
	Game::GetWindow().setActive(true);
	while(true)
	{
		lobby.Show(Game::GetWindow());
	}
}

void multiplayer::setGameStarted(bool _isStarted)
{
	gameStarted = _isStarted;
}


void multiplayer::setExitingLobby(bool _isExiting)
{
	exitingLobby = _isExiting;
}

bool multiplayer::getGameStarted()
{
	return gameStarted;
}

void multiplayer::freePeers()
{
	//std::cout << "size: " << (int) allPeers.size() << std::endl;

	//std::cout << "starting freeee" << std::endl;
/*
	for(std::map<int,peer*>::iterator it = allPeers.begin(); it != allPeers.end(); ++it)//locking up on this line
	{
		std::cout << "freeing: " << it->first << std::endl;

		delete it->second;

		std::cout << "freed" << std::endl;
		
	}
*/
	utils::mutex.lock();

	std::cout << "declare iterator" << std::endl;
	std::map<int,peer*>::iterator it;


	std::cout << "init to beginning" << std::endl;
	it = allPeers.begin();
	
	std::cout << "starting while" << std::endl;
	while( it!=allPeers.end() )
	{
		std::cout << "freeing: " << it->first << std::endl;

		delete it->second;

		std::cout << "freed" << std::endl;

		it++;

		std::cout << "inc" << std::endl;
	}



	std::cout << "done freeing" << std::endl;

	allPeers.clear();

	std::cout << "done clearing" << std::endl;

	utils::mutex.unlock();
}

void multiplayer::freeLobyIcons()
{
	lobby.freeIcons();
}

void multiplayer::unbindSockets()
{
	listener.unbind();
}

void multiplayer::copyPeers()
{
	for(std::map<int,peer*>::iterator it = allPeers.begin(); it != allPeers.end(); ++it)
	{
		if(!it->second->isBot)
		{
			peer* copy = new peer;
			copy->address= it->second->address;
			reJoinPeers.insert( std::pair<int,peer*>(it->first,copy) );
		}
	}
}

void multiplayer::sendReJoin()
{
		
	std::cout << reJoinPeers.size() << std::endl;		

	sf::Packet packet;
	networkEvent reJoin;

	reJoin.myType = networkEvent::reJoin;

	packet << reJoin;

	for(std::map<int,peer*>::iterator it = reJoinPeers.begin(); it != reJoinPeers.end(); ++it)
	{
		if ( it->first != myPlayerNum)
		{
			std::cout << "sending rejoin" << std::endl;
			Game::sender.send(packet,it->second->address,multiplayer::sendPort);
		}

		sf::Time waitTime = sf::milliseconds(25);
		sf::sleep(waitTime);

	}
}

void multiplayer::destroyPeerCopy()
{
	for(std::map<int,peer*>::iterator it = reJoinPeers.begin(); it != reJoinPeers.end(); ++it)
	{
		delete it->second;
	}
	reJoinPeers.clear();
}

void multiplayer::addBot()
{
	int newPlayerNum = playerCounter;
	playerCounter++;

	// new player
	std::cout << "new player" << std::endl;

	mechSpec spec;
	std::string name = "bot";
	
	peer* addingPeer = new peer(name, spec, true);
	allPeers.insert( std::pair<int,peer*>(newPlayerNum,addingPeer) );

	// add client icon to lobby
	lobby.addIcon(newPlayerNum,name);

	sf::Packet outPacket;
	lobbyEvent joinEvent;
	joinEvent.myType = lobbyEvent::playerJoin;
	joinEvent.playerNum = newPlayerNum;
	outPacket << joinEvent;
	sendToOthersTCP(newPlayerNum,outPacket);
	outPacket.clear();
}

void multiplayer::updateBots()
{
	for(std::map<int,peer*>::iterator it = allPeers.begin(); it != allPeers.end(); ++it)
	{
		if(it->second->isBot)
			it->second->runAi();
	}
}

sf::Packet multiplayer::latestPacket;
multiplayer::PeerState multiplayer::_peerState;

sf::IpAddress multiplayer::serverAddr;

unsigned short multiplayer::sendPort;
unsigned short multiplayer::listenPort;
unsigned short multiplayer::tcpPort;


int multiplayer::myPlayerNum;
int multiplayer::playerCounter;

lobby multiplayer::lobby;

std::map<int,peer*> multiplayer::allPeers;
std::map<int,peer*> multiplayer::reJoinPeers;

bool multiplayer::gameStarted;
bool multiplayer::exitingLobby;
bool multiplayer::quittingFromLobby;

sf::UdpSocket multiplayer::listener;

std::map<int,sf::TcpSocket*> multiplayer::clients;


sf::TcpSocket* multiplayer::clientLobbySock;

