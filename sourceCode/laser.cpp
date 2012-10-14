#include "laser.h"
#include "Game.h"
#include "box.h"
#include "images.h"
#include "player.h"
#include "multiplayer.h"
#include "networkEvent.h"
#include "packet.h"
#include "utils.h"

#include "GameObjectManager.h"

#include <assert.h>
#include <sstream>

laser::laser()
{ 	
}

laser::laser(float x, float y, player* _player)
{ 
	armStat stats = _player->getArmStat();

	loadFromTex(*images::allProjectiles.find(stats.projName)->second);
	assert(IsLoaded());

	_player->setArmFacing(sf::Vector2f(x,y));

	sf::Vector2f armPos = _player->getArmTip();

	velX = x - armPos.x;
	velY = y - armPos.y;

	float mag = sqrt(velX*velX+velY*velY);

	velX = velX / mag;
	velY = velY / mag;

	ownerPlayerNum = _player->getPlayerNum();
	boxDamage = stats.boxDamage;
	playerDamage = stats.playerDamage;

	this->GetSprite().setOrigin(0,this->GetHeight()/2);
}


laser::~laser()
{
}


void laser::setVel(float x, float y)
{
	velX = x;
	velY = y;
}



bool laser::checkCollide()
{


	// check out of screen bounds
	if(  target.x < 0 ||
		 target.x > Game::mapWidth*Game::GRID_WIDTH ||
		 target.y < 0 ||
		 target.y > Game::mapHeight*Game::GRID_HEIGHT 
	  )
	{
			return true;
	}



	// check collide with other player
	//for(int i=0; i < multiplayer::numberOfPlayers; i++)
	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it !=  multiplayer::allPeers.end(); ++it)
	{
		if(  it->second->hisPlayer->containsPoint(target) && 
			 ownerPlayerNum != it->second->hisPlayer->getPlayerNum()  && 
			 !it->second->hisPlayer->getIsRespawning()  )
		{
				bool shouldDamage;
				bool onSameTeam;
				onSameTeam = (it->second->team == multiplayer::allPeers.find(this->ownerPlayerNum)->second->team);
				shouldDamage =  (multiplayer::_peerState == multiplayer::server) && 
								(       (Game::myGameType == Game::ffa) || (  (Game::myGameType == Game::teams)&&(!onSameTeam || Game::ffIsOn)  )       );


				if(shouldDamage && it->second->hisPlayer->laserTicker.getElapsedTime().asSeconds() > 0.1f)
				{
					it->second->hisPlayer->laserTicker.restart();

					it->second->hisPlayer->takeDamage(playerDamage);
					sf::Packet outPacket;
					outPacket.clear();
					networkEvent currentNetEvent;

					currentNetEvent.myType = networkEvent::playerHit;
					currentNetEvent.playerNumber = it->first;
					currentNetEvent.playerDamage = playerDamage;

					outPacket << currentNetEvent;
					multiplayer::sendToClients(outPacket);
					outPacket.clear();
				}

				return true;
		}
	}

	int i = (int) floor(target.x / Game::GRID_WIDTH);
	int j = (int) floor(target.y / Game::GRID_HEIGHT);

	if (i < 0)
		i = 0;
	if (i >= Game::mapWidth)
		i = Game::mapWidth-1;

	if (j < 0)
		j = 0;
	if (j >= Game::mapHeight)
		j = Game::mapHeight-1;

	if(Game::gameGrid[i][j].getCollide())
	{
		
		if(multiplayer::_peerState == multiplayer::server && Game::gameGrid[i][j].laserTicker.getElapsedTime().asSeconds() > 0.1f)
		{
			Game::gameGrid[i][j].laserTicker.restart();

			Game::gameGrid[i][j].takeDamage(boxDamage);
			sf::Packet outPacket;
			outPacket.clear();
			networkEvent currentNetEvent;

			currentNetEvent.myType = networkEvent::boxHit;
			currentNetEvent.boxX = i;
			currentNetEvent.boxY = j;
			currentNetEvent.boxDamage = boxDamage;

			outPacket << currentNetEvent;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}
	

		return true;
	}

	return false;
}

void laser::Update(float elapsedTime)
{
	

	sf::Vector2f armPos = multiplayer::allPeers.find(this->ownerPlayerNum)->second->hisPlayer->getArmTip();
	float angle = multiplayer::allPeers.find(this->ownerPlayerNum)->second->hisPlayer->getArmRotation();
	
	const float pi = 3.14159265f;
	velX = cos(angle*pi/180)*4;
	velY = sin(angle*pi/180)*4;

	if(ownerPlayerNum == multiplayer::myPlayerNum)
	{
		if( !sf::Mouse::isButtonPressed(sf::Mouse::Left) || 
			multiplayer::allPeers.find(this->ownerPlayerNum)->second->hisPlayer->getIsDead() || 
			multiplayer::allPeers.find(this->ownerPlayerNum)->second->hisPlayer->getIsRespawning()  )
		{
			
			destroyProjectile();


			sf::Packet outPacket;
			networkEvent currentNetEvent;

			currentNetEvent.myType = networkEvent::destroyProjectile;
			currentNetEvent.projNum = this->projNum;
			
			outPacket << currentNetEvent;

			if(multiplayer::_peerState == multiplayer::client)
			{
				Game::sender.send(outPacket,multiplayer::serverAddr,multiplayer::sendPort);
				outPacket.clear();
			}
			else if(multiplayer::_peerState == multiplayer::server)
			{
				multiplayer::sendToClients(outPacket);
				outPacket.clear();
			}

		}
	}

	target = armPos;
	while (!checkCollide() && !getShouldRemove() )
	{
		target.x+=velX;
		target.y+=velY;
	}
	
}

void laser::destroyProjectile()
{
	utils::mutex.lock();

	multiplayer::allPeers.find(this->ownerPlayerNum)->second->hisPlayer->isLasering = false;

	velX=0;
	velY=0;

	setShouldRemove(true);

	utils::mutex.unlock();
}

void laser::Draw(sf::RenderWindow & renderWindow)
{
	sf::Vector2f armPos = multiplayer::allPeers.find(this->ownerPlayerNum)->second->hisPlayer->getArmTip();
	float angle = multiplayer::allPeers.find(this->ownerPlayerNum)->second->hisPlayer->getArmRotation();

	float dx = target.x - armPos.x;
	float dy = target.y - armPos.y;

	float dist = sqrt(dx*dx+dy*dy);

	this->scale(dist/this->GetWidth(),1.f);
	this->SetPosition(armPos.x,armPos.y);
	this->setRotation(angle);

	renderWindow.draw(GetSprite());
	
}
