#include "packet.h"
#include "networkEvent.h"
#include "mechSpec.h"
#include "multiplayer.h"
#include "lobbyEvent.h"
#include "Game.h"

sf::Packet& operator <<(sf::Packet& Packet, const VisibleGameObject& _vgo)
{
    return Packet << _vgo.GetPosition().x << _vgo.GetPosition().y;
}

sf::Packet& operator >>(sf::Packet& Packet, VisibleGameObject& _vgo)
{
    float x;
	float y;
	Packet >> x >> y;
	_vgo.SetPosition(x,y);
	return Packet;
}

sf::Packet& operator <<(sf::Packet& Packet, const networkEvent& netEvent)
{
	
	if(netEvent.myType == networkEvent::moveStart || netEvent.myType == networkEvent::moveStop)
	{
		return Packet << (int) (netEvent.myType) << (int) (netEvent.myMoveKey) << netEvent.playerNumber;
	}
	if(netEvent.myType == networkEvent::shotFired)
	{
		return Packet << (int) (netEvent.myType) << netEvent.mouseX << netEvent.mouseY << netEvent.playerNumber << netEvent.projNum;
	}
	if(netEvent.myType == networkEvent::missileFired)
	{
		return Packet << (int) (netEvent.myType) << netEvent.mouseX << netEvent.mouseY << netEvent.playerNumber << netEvent.projNum;
	}
	if(netEvent.myType == networkEvent::boxHit)
	{
		return Packet << (int) (netEvent.myType) << netEvent.boxX << netEvent.boxY << netEvent.boxDamage;
	}
	if(netEvent.myType == networkEvent::playerHit)
	{
		return Packet << (int) (netEvent.myType) << netEvent.playerNumber << netEvent.playerDamage;
	}
	if(netEvent.myType == networkEvent::sync)
	{
		return Packet << (int) (netEvent.myType) << netEvent.playerNumToSync << netEvent.playerX << netEvent.playerY << netEvent.velX << netEvent.velY;
	}
	if(netEvent.myType == networkEvent::lostFocus)
	{
		return Packet << (int) (netEvent.myType) << multiplayer::myPlayerNum;
	}
	if(netEvent.myType == networkEvent::armRotate)
	{
		return Packet << (int) (netEvent.myType) << netEvent.angle << netEvent.playerNumber;
	}
	if(netEvent.myType == networkEvent::clientLeave)
	{
		return Packet << (int) (netEvent.myType) << multiplayer::myPlayerNum;
	}
	if(netEvent.myType == networkEvent::serverLeave)
	{
		return Packet << (int) (netEvent.myType);
	}
	if(netEvent.myType == networkEvent::serverLeave)
	{
		return Packet << (int) (netEvent.myType);
	}
	if(netEvent.myType == networkEvent::gameOver)
	{
		return Packet << (int) (netEvent.myType) << netEvent.winnerNum;
	}
	if(netEvent.myType == networkEvent::reJoin)
	{
		return Packet << (int) (netEvent.myType);
	}
	if(netEvent.myType == networkEvent::destroyProjectile)
	{
		return Packet << (int) (netEvent.myType) << netEvent.projNum << multiplayer::myPlayerNum;
	}
	if(netEvent.myType == networkEvent::knockBack)
	{
		return Packet   << (int) (netEvent.myType) << netEvent.playerNumber 
						<< netEvent.knockDirecion.x << netEvent.knockDirecion.y
						<< netEvent.knockTime.asSeconds();
	}

	if(netEvent.myType == networkEvent::chargeStart)
	{
		return Packet << (int) (netEvent.myType) << netEvent.playerNumber;
	}

	return Packet;
}


sf::Packet& operator <<(sf::Packet& Packet, const mechSpec& mechSpec)
{
	Packet << mechSpec.legName << mechSpec.bodyName << mechSpec.armName << mechSpec.shoulderName;
	
	Packet << mechSpec.equipment.size();

	for(std::map<int,std::string>::const_iterator it = mechSpec.equipment.begin(); it !=  mechSpec.equipment.end(); it++)
	{
		Packet << (it->first) << (it->second);
	}

	return Packet;
}

sf::Packet& operator >>(sf::Packet& Packet, mechSpec& mechSpec)
{
	std::string legChoice, bodyChoice, armChoice, shoulderChoice;
	Packet >> legChoice >> bodyChoice >> armChoice >> shoulderChoice;

	mechSpec.legName = legChoice;
	mechSpec.bodyName = bodyChoice;
	mechSpec.armName = armChoice;
	mechSpec.shoulderName = shoulderChoice;

	int numEquips;
	Packet>>numEquips;
	for(int i = 0; i<numEquips; i++)
	{
		std::string equipName;
		int num;
		Packet>>num;
		Packet>>equipName;

		mechSpec.equipment.insert(std::pair<int,std::string>(num,equipName));
		//std::cout << equipName << std::endl;
	}

	return Packet;
}


sf::Packet& operator <<(sf::Packet& Packet, const lobbyEvent& lobEv)
{
	
	
	if(lobEv.myType == lobbyEvent::newConnection)
	{
		Packet << (int) (lobEv.myType) << lobEv.playerNum << Game::lives << (int) Game::myGameType << Game::ffIsOn << Game::selectedMap << multiplayer::allPeers.size();
		for(std::map<int,peer*>::iterator it2 = multiplayer::allPeers.begin(); it2 != multiplayer::allPeers.end(); ++it2)
			Packet << it2->first <<  it2->second->name << it2->second->spec << it2->second->team;
	}

	else if(lobEv.myType == lobbyEvent::playerJoin)
	{
		Packet << (int) (lobEv.myType);
		peer* newPeer = multiplayer::allPeers.find(lobEv.playerNum)->second;
		Packet << lobEv.playerNum <<  newPeer->name << newPeer->spec;
	}

	else if(lobEv.myType == lobbyEvent::gameStart)
	{
		Packet << (int) (lobEv.myType);
	}

	else if(lobEv.myType == lobbyEvent::playerLeave)
	{
		Packet << (int) (lobEv.myType);
		Packet << lobEv.playerNum;
	}

	else if(lobEv.myType == lobbyEvent::changeLives)
	{
		Packet << (int) (lobEv.myType);
		Packet << Game::lives;
	}

	else if(lobEv.myType == lobbyEvent::changeSpec)
	{
		Packet << (int) (lobEv.myType);
		Packet << lobEv.playerNum;
		Packet << Game::activeSpec;
	}

	else if(lobEv.myType == lobbyEvent::joinRequest)
	{
		Packet << (int) (lobEv.myType);
		Packet << Game::activeSpec;
		Packet << Game::myName;
	}

	else if(lobEv.myType == lobbyEvent::changeGameType)
	{
		Packet << (int) (lobEv.myType);
		Packet << lobEv.gameTypeNum;
	}

	else if(lobEv.myType == lobbyEvent::changeTeam)
	{
		Packet << (int) (lobEv.myType);
		Packet << lobEv.playerNum;
		Packet << lobEv.teamNum;
	}
	else if(lobEv.myType == lobbyEvent::changeFF)
	{
		Packet << (int) (lobEv.myType);
		Packet << Game::ffIsOn;
	}
	else if(lobEv.myType == lobbyEvent::changeMap)
	{
		Packet << (int) (lobEv.myType);
		Packet << lobEv.mapName;
	}
	else if(lobEv.myType == lobbyEvent::launchMap)
	{
		Packet << (int) (lobEv.myType);
		Packet << lobEv.mapHash;
	}
	else if(lobEv.myType == lobbyEvent::loadedMap)
	{
		Packet << (int) (lobEv.myType);
		Packet << lobEv.playerNum;
		Packet << lobEv.mapReady;
	}
	else if(lobEv.myType == lobbyEvent::startTranfer)
	{
		Packet << (int) (lobEv.myType);
		Packet << lobEv.transferFilePath;
	}

	return Packet;
}