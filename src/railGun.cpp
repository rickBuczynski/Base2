#include "railGun.h"
#include "Game.h"
#include "box.h"
#include "images.h"
#include "player.h"
#include "multiplayer.h"
#include "networkEvent.h"
#include "packet.h"
#include "utils.h"
#include "sounds.h"

#include "GameObjectManager.h"

#include <assert.h>
#include <sstream>

railGun::railGun()
{ 	
}

railGun::railGun(float x, float y, player* _player)
{ 
	armStat stats = _player->getArmStat();

	loadFromTex(*images::allProjectiles.find(stats.projName)->second);
	assert(IsLoaded());

	_player->setArmFacing(sf::Vector2f(x,y));

	sf::Vector2f armPos = _player->getArmTip();

	ownerPlayerNum = _player->getPlayerNum();
	boxDamage = stats.boxDamage;
	playerDamage = stats.playerDamage;

	this->GetSprite().setOrigin(0,this->GetHeight()/2);
	float angle = _player->getArmRotation();
	
	const float pi = 3.14159265f;
	velX = cos(angle*pi/180)*this->GetWidth()/2;
	velY = sin(angle*pi/180)*this->GetWidth()/2;

	SetPosition(armPos.x, armPos.y);
	setRotation(angle);

	startPos = armPos;
	startAngle = angle;

	length = 0;
	while (!checkCollide() && !getShouldRemove() )
	{
		GetSprite().move(velX , velY );
		length++;
	}

	fadeTimer.restart();

	sf::Sound* snd = new sf::Sound;
	snd->setBuffer(*sounds::shots.find(stats.soundName)->second);
	Game::mySoundManager.Add(snd);
	snd->play();
}


railGun::~railGun()
{
}


void railGun::setVel(float x, float y)
{
	velX = x;
	velY = y;
}



bool railGun::checkCollide()
{
	sf::Rect<float> proj_rect = this->GetBoundingRect();

	sf::Vector2f pos;

	if(velX > 0)
		pos.x = proj_rect.left + proj_rect.width;
	else
		pos.x = proj_rect.left;

	if(velY > 0)
		pos.y = proj_rect.top + proj_rect.height;
	else
		pos.y = proj_rect.top;

	// check out of screen bounds
	if(  pos.x < 0 ||
		 pos.x > Game::mapWidth*Game::GRID_WIDTH ||
		 pos.y < 0 ||
		 pos.y > Game::mapHeight*Game::GRID_HEIGHT 
	  )
	{
			return true;
	}



	// check collide with other player
	//for(int i=0; i < multiplayer::numberOfPlayers; i++)
	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it !=  multiplayer::allPeers.end(); ++it)
	{
		if(  it->second->hisPlayer->containsPoint(pos) && 
			 ownerPlayerNum != it->second->hisPlayer->getPlayerNum()  && 
			 !it->second->hisPlayer->getIsRespawning()  )
		{
			bool shouldDamage;
			bool onSameTeam;
			onSameTeam = (it->second->team == multiplayer::allPeers.find(this->ownerPlayerNum)->second->team);
			shouldDamage =  (multiplayer::_peerState == multiplayer::server) && 
							(       (Game::myGameType == Game::ffa) || (  (Game::myGameType == Game::teams)&&(!onSameTeam || Game::ffIsOn)  )       );
			
			if(shouldDamage)
			{
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

	int i = (int) floor(pos.x / Game::GRID_WIDTH);
	int j = (int) floor(pos.y / Game::GRID_HEIGHT);

	if(Game::gameGrid[i][j].getCollide())
	{
		
		if(multiplayer::_peerState == multiplayer::server)
		{
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

void railGun::Update(float elapsedTime)
{
	SetPosition(startPos.x, startPos.y);
	setRotation(startAngle);

	float time = fadeTimer.getElapsedTime().asSeconds();

	this->GetSprite().setColor(sf::Color(255,255,255,(int)floor( 255*(1-time/0.5f) ) ) );

	if(multiplayer::_peerState == multiplayer::server && time > 0.5f)
	{
		destroyProjectile();


		sf::Packet outPacket;
		networkEvent currentNetEvent;

		currentNetEvent.myType = networkEvent::destroyProjectile;
		currentNetEvent.projNum = this->projNum;
		
		outPacket << currentNetEvent;

		multiplayer::sendToClients(outPacket);
		outPacket.clear();
	}
	
}

void railGun::destroyProjectile()
{
	utils::mutex.lock();

	velX=0;
	velY=0;

	setShouldRemove(true);

	utils::mutex.unlock();
}

void railGun::Draw(sf::RenderWindow & renderWindow)
{
	for(int i=0; i<length; i++)
	{
		GetSprite().move(velX , velY );
		renderWindow.draw(GetSprite());
	}
}
