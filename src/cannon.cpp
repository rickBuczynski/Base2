#include "cannon.h"
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

cannon::cannon()
{ 	
}

cannon::cannon(float x, float y, player* _player)
{ 
	armStat stats = _player->getArmStat();

	loadFromTex(*images::allProjectiles.find(stats.projName)->second);
	assert(IsLoaded());

	_player->setArmFacing(sf::Vector2f(x,y));

	float chargeTime = _player->chargeTimer.getElapsedTime().asSeconds();

	if(chargeTime > 2)
		chargeTime = 2;


	this->scale(chargeTime/0.5f,chargeTime/0.5f);


	accX = 0;
	accY = 0;

	sf::Vector2f armPos = _player->getArmTip();

	float angle = _player->getArmRotation();

	const float pi = 3.14159265f;
	velX = cos(angle*pi/180)*stats.projectileSpeed;
	velY = sin(angle*pi/180)*stats.projectileSpeed;

	ownerPlayerNum = _player->getPlayerNum();
	boxDamage = (int)ceil( (float)stats.boxDamage*chargeTime/0.5f );
	playerDamage = (int)ceil( (float)stats.playerDamage*chargeTime/0.5f );

	SetPosition(armPos.x, armPos.y-GetBoundingRect().height/2);
}


cannon::~cannon()
{
}


void cannon::setVel(float x, float y)
{
	velX = x;
	velY = y;
}

void cannon::setAcc(float x, float y)
{
	accX = x;
	accY = y;
}


bool cannon::checkCollide(float & velX, float & velY, float elapsedTime)
{

	sf::Rect<float> proj_rect = this->GetBoundingRect();


	// check out of screen bounds
	if(  proj_rect.left < 0 ||
		 (proj_rect.left + proj_rect.width) > Game::mapWidth*Game::GRID_WIDTH ||
		 proj_rect.top < 0 ||
		 (proj_rect.top + proj_rect.height) > Game::mapHeight*Game::GRID_HEIGHT 
	  )
	{
			destroyProjectile();
			return true;
	}


	// check collide with other player
	//for(int i=0; i < multiplayer::numberOfPlayers; i++)
	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it !=  multiplayer::allPeers.end(); ++it)
	{
		if(  it->second->hisPlayer->isIntersecting(*this,0,0,0) && 
			 ownerPlayerNum != it->second->hisPlayer->getPlayerNum()  && 
			 !it->second->hisPlayer->getIsRespawning()  )
		{
				destroyProjectile();

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


	int i_left;
	int j_top;
	int i_right;
	int j_bottom;

	i_left =   (int) floor(proj_rect.left / Game::GRID_WIDTH);
	j_top =    (int) floor(proj_rect.top / Game::GRID_HEIGHT);
	i_right =  (int) floor((proj_rect.left + proj_rect.width) / Game::GRID_WIDTH);
	j_bottom = (int) floor((proj_rect.top + proj_rect.height) / Game::GRID_HEIGHT);

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
	for(i = i_left; i <= i_right; i++)
	{
		for(j = j_top; j <= j_bottom; j++)
		{

			// outer if is performance optimization
			// only check outside of object not inside grid
			if(i == i_left || i == i_right || j == j_top || j == j_bottom)
			{
				if(Game::gameGrid[i][j].getCollide())
				{
					if(Game::gameGrid[i][j].GetBoundingRect().intersects(proj_rect)  )
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

						destroyProjectile();
						return true;
					}
				}
			}


		}
	}

	return false;
}

void cannon::Update(float elapsedTime)
{
	velY += accY * elapsedTime;
	velX += accX * elapsedTime;

	//sf::Vector2f pos = this->GetPosition();

	// dummies allow checkCollide to only set x to 0 if moving causes an x collsion
	// and y to 0 if moving causes a y collision
	bool isColliding = this->checkCollide(velX, velY, elapsedTime);

	if ( !isColliding )
		GetSprite().move(velX * elapsedTime, velY * elapsedTime);
}

void cannon::destroyProjectile()
{
	utils::mutex.lock();

	velX=0;
	velY=0;

	Game::allProjectiles.erase(this->projNum);
	setShouldRemove(true);

	utils::mutex.unlock();
}