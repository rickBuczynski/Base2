#include "missile.h"
#include "Game.h"
#include "box.h"
#include "images.h"
#include "player.h"
#include "multiplayer.h"
#include "networkEvent.h"
#include "packet.h"
#include "effect.h"
#include "utils.h"
#include "sounds.h"

#include "GameObjectManager.h"

#include <assert.h>
#include <sstream>

missile::missile()
{ 	
}

missile::missile(float x, float y, player* _player)
{ 
	shoulderStat stat = _player->getShoulderStat();

	loadFromTex(*images::allProjectiles.find(stat.projName)->second);
	assert(IsLoaded());

	if(stat.myWeaponType == shoulderStat::rpg)
		accScalar = 1500.f;
	else
		accScalar = 0;


	accX = 0;
	accY = 0;

	sf::Vector2f pos = _player->getShoulderCenter();

	velX = x - pos.x;
	velY = y - pos.y;

	float mag = sqrt(velX*velX+velY*velY);

	velX = velX / mag * stat.speed;
	velY = velY / mag * stat.speed;

	speed = stat.speed;

	ownerPlayerNum = _player->getPlayerNum();
	boxDamage = stat.boxDamage;
	playerDamage = stat.playerDamage;
	expRadius = stat.explRadius;
	knockForce = stat.force;
	knockTime = stat.knockTime;
	explTime = stat.explTime;
	isHoming = stat.isHoming;
	explSound = stat.explSound;

	SetPosition(pos.x, pos.y);
	rotateToFace(x,y);

	effectPos.x = 0;
	effectPos.y = 0;

	sf::Sound* snd = new sf::Sound;
	snd->setBuffer(*sounds::shots.find(stat.launchSound)->second);
	Game::mySoundManager.Add(snd);
	snd->play();
}


missile::~missile()
{
}


void missile::setVel(float x, float y)
{
	velX = x;
	velY = y;
}

void missile::setAcc(float x, float y)
{
	accX = x;
	accY = y;
}


bool missile::checkCollide(float & velX, float & velY, float elapsedTime)
{

	sf::Rect<float> proj_rect	(	this->GetPosition().x + velX*elapsedTime, 
									this->GetPosition().y + velY*elapsedTime,
									this->GetWidth(),
									this->GetHeight() 
								);

	sf::ConvexShape trueRect = this->getTrueRect();

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
	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it !=  multiplayer::allPeers.end(); ++it)
	{
		if(  this->isIntersecting(*it->second->hisPlayer) && 
			 ownerPlayerNum != it->second->hisPlayer->getPlayerNum()  && 
			 !it->second->hisPlayer->getIsRespawning()  )
		{
				destroyProjectile();
				return true;
		}
	}



	float maxX = trueRect.getPoint(0).x;
	float maxY = trueRect.getPoint(0).y;
	float minX = maxX;
	float minY = maxY;
	for (unsigned int i=1; i<trueRect.getPointCount(); i++)
	{
		sf::Vector2f point = trueRect.getPoint(i);

		if(point.x > maxX)
			maxX = point.x;

		if(point.y > maxY)
			maxY = point.y;

		if(point.x < minX)
			minX = point.x;

		if(point.y < minY)
			minY = point.y;
	}


	int i_left;
	int j_top;
	int i_right;
	int j_bottom;

	i_left =   (int) floor(minX / Game::GRID_WIDTH);
	j_top =    (int) floor(minY / Game::GRID_HEIGHT);
	i_right =  (int) floor(maxX / Game::GRID_WIDTH);
	j_bottom = (int) floor(maxY / Game::GRID_HEIGHT);

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
			if( Game::gameGrid[i][j].getCollide() )
			{
				if( this->isIntersecting (Game::gameGrid[i][j]) )
				{
					destroyProjectile();
					return true;
				}
			}
		}
	}

	return false;
}

void missile::Update(float elapsedTime)
{
	velY += accY * elapsedTime;
	velX += accX * elapsedTime;

	speed += accScalar * elapsedTime;
	
	float mag = sqrt(velX*velX+velY*velY);
	velX = velX / mag * speed;
	velY = velY / mag * speed;

	float angle = atan( velY/velX );

	const float pi = 3.14159265f;

	angle = angle*180/pi;

	if(velX < 0)
		angle+=180;

	setRotation(angle);

	//sf::Vector2f pos = this->GetPosition();

	// dummies allow checkCollide to only set x to 0 if moving causes an x collsion
	// and y to 0 if moving causes a y collision
	bool isColliding = this->checkCollide(velX, velY, elapsedTime);

	if ( !isColliding )
		GetSprite().move(velX * elapsedTime, velY * elapsedTime);

	if( isHoming && multiplayer::allPeers.size() > 1)
		homeTarget();


	float sqDist = utils::calcSqDistance(this->getTip(),effectPos);

	//std::cout << sqDist << std::endl;
	if( sqDist > 1000.f)
	{
		effectPos = ( this->getTrueRect().getPoint(0)+this->getTrueRect().getPoint(3) )*0.5f;
		sf::Time fadeTime = sf::seconds(0.2f);
		effect* _effect = new effect(effectPos.x, effectPos.y, 8,8,4,1,fadeTime,images::explosions::smoke,sf::Vector2f(0,0));
		Game::GetGameObjectManager().Add(_effect);
	}
}

void missile::generateExplosion()
{
	effect* _effect = new effect(this->GetPosition().x, this->GetPosition().y, 128,128,8,8,explTime,images::explosions::missile_explosion,sf::Vector2f(0,0));
	Game::GetGameObjectManager().Add(_effect);

	if(multiplayer::_peerState == multiplayer::server)
	{

		sf::Packet outPacket;
		outPacket.clear();
		networkEvent currentNetEvent;

		currentNetEvent.myType = networkEvent::destroyProjectile;
		currentNetEvent.projNum = this->projNum;

		outPacket << currentNetEvent;
		multiplayer::sendToClients(outPacket);
		outPacket.clear();

		sf::Vector2f explCenter = this->GetPosition();

		for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it !=  multiplayer::allPeers.end(); ++it)
		{
			if( !it->second->hisPlayer->getIsDead() && !it->second->hisPlayer->getIsRespawning() )
			{
			
				sf::FloatRect legRect = it->second->hisPlayer->GetBoundingRect();
				sf::FloatRect bodyRect = it->second->hisPlayer->getBody().GetBoundingRect();
				sf::Vector2f playerPos;
				sf::Vector2f massCenter(legRect.left+legRect.width/2,legRect.top);


				float left = std::min(legRect.left,bodyRect.left);
				float right = std::max(legRect.left+legRect.width,bodyRect.left+bodyRect.width);
				float top = std::min(legRect.top,bodyRect.top);
				float bottom = std::max(legRect.top+legRect.height,bodyRect.top+bodyRect.height);

				// use closest edge of player to explosion as their position
				if ( explCenter.x < left )
					playerPos.x = left;
				else if ( explCenter.x > right )
					playerPos.x = right;
				else
					playerPos.x = explCenter.x;

				if ( explCenter.y < top )
					playerPos.y = top;
				else if ( explCenter.y > bottom )
					playerPos.y = bottom;
				else
					playerPos.y = explCenter.y;

				// knockback vector
				sf::Vector2f knockDir(massCenter.x-explCenter.x , massCenter.y-explCenter.y);
				float mag = sqrt(knockDir.x*knockDir.x+knockDir.y*knockDir.y); 
				knockDir.x = knockDir.x/mag;
				knockDir.y = knockDir.y/mag;
				knockDir.x*=knockForce;
				knockDir.y*=knockForce;

				float sqDistance = pow( (explCenter.x-playerPos.x), 2) + pow( (explCenter.y-playerPos.y), 2);
				float sqRadius = pow( expRadius, 2);
				int damageTaken = (int) floor (playerDamage*(1-sqDistance/sqRadius) );

				bool shouldDamage;
				bool onSameTeam;
				bool isMe;
				onSameTeam = (it->second->team == multiplayer::allPeers.find(this->ownerPlayerNum)->second->team);
				isMe = (this->ownerPlayerNum == it->first);
				shouldDamage =  ( damageTaken > 0) && 
								(       (Game::myGameType == Game::ffa) || (  (Game::myGameType == Game::teams)&&(!onSameTeam || Game::ffIsOn || isMe)  )       );


				if(shouldDamage)
				{
					std::cout << it->second->name << " takes: " << damageTaken << std:: endl;

					it->second->hisPlayer->knockBack(knockDir,knockTime);
					sf::Packet outPacket;
					outPacket.clear();
					networkEvent knockEvent;

					knockEvent.myType = networkEvent::knockBack;
					knockEvent.playerNumber = it->first;
					knockEvent.knockDirecion = knockDir;
					knockEvent.knockTime = knockTime;

					outPacket << knockEvent;
					multiplayer::sendToClients(outPacket);
					outPacket.clear();

					it->second->hisPlayer->takeDamage(damageTaken);
					outPacket.clear();
					networkEvent currentNetEvent;

					currentNetEvent.myType = networkEvent::playerHit;
					currentNetEvent.playerNumber = it->first;
					currentNetEvent.playerDamage = damageTaken;

					outPacket << currentNetEvent;
					multiplayer::sendToClients(outPacket);
					outPacket.clear();
				}

			}
		}

		int i_left;
		int j_top;
		int i_right;
		int j_bottom;

		i_left =   (int) floor(  (explCenter.x-expRadius) / Game::GRID_WIDTH);
		j_top =    (int) floor(  (explCenter.y-expRadius) / Game::GRID_HEIGHT);
		i_right =  (int) ceil(  (explCenter.x+expRadius) / Game::GRID_WIDTH);
		j_bottom = (int) ceil(  (explCenter.y+expRadius) / Game::GRID_HEIGHT);

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
				
				float sqDistance;
				sqDistance = pow( (Game::gameGrid[i][j].GetPosition().x+Game::GRID_WIDTH/2-explCenter.x)   ,  2   ) + 
							 pow( (Game::gameGrid[i][j].GetPosition().y+Game::GRID_HEIGHT/2-explCenter.y)  ,  2   ) ;


				float sqRadius = pow( expRadius, 2);

				int damageTaken = (int) floor (boxDamage*(1-sqDistance/sqRadius) );

				if( damageTaken > 0 && Game::gameGrid[i][j].getIsDrawn() )
				{
					Game::gameGrid[i][j].takeDamage(damageTaken);
					sf::Packet outPacket;
					outPacket.clear();
					networkEvent currentNetEvent;

					currentNetEvent.myType = networkEvent::boxHit;
					currentNetEvent.boxX = i;
					currentNetEvent.boxY = j;
					currentNetEvent.boxDamage = damageTaken;

					outPacket << currentNetEvent;
					multiplayer::sendToClients(outPacket);
					outPacket.clear();	
				}
					
				
			


			}
		}
	}
}

void missile::destroyProjectile()
{
	utils::mutex.lock();

	sf::Sound* snd = new sf::Sound;
	snd->setBuffer(*sounds::explosions.find(explSound)->second);
	Game::mySoundManager.Add(snd);
	snd->play();

	generateExplosion();

	velX=0;
	velY=0;
	
	Game::allProjectiles.erase(this->projNum);
	setShouldRemove(true);

	utils::mutex.unlock();
}



bool missile::isIntersecting(const VisibleGameObject & vgo)
{
	sf::ConvexShape first = this->getTrueRect();
	sf::ConvexShape second = vgo.getTrueRect();

	std::list<sf::Vector2f> axes;

	utils::addAxes(axes,first);
	utils::addAxes(axes,second);

	for (std::list<sf::Vector2f>::iterator it = axes.begin(); it != axes.end(); ++it)
	{
		utils::projection p1 = utils::project(*it,first);
		utils::projection p2 = utils::project(*it,second);

		// if there is a projection that doesn't overlap, the shapes don't intersect
		if (!p1.isOverlaping(p2)) 
			return false;
	}
	//all projections overlap, the shapes must intersect
	return true;
}

bool missile::isIntersecting(const player & ply)
{
	if(    this->isIntersecting(  (VisibleGameObject)ply  )    )
		return true;
	else if ( this->isIntersecting(  ply.getBody()  )   )
		return true;
	else
		return false;
}



void missile::homeTarget()
{
	sf::Vector2f missPos = this->GetPosition();
	sf::Vector2f playPos;
	float sqDist;
	float minDist;
	int targetNum;

	bool foundTarget = false;

	minDist = (float) Game::mapWidth*Game::GRID_WIDTH*1000;

	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it !=  multiplayer::allPeers.end(); ++it)
	{
		bool onSameTeam = (it->second->team == multiplayer::allPeers.find(this->ownerPlayerNum)->second->team);
		bool shouldHome = (Game::myGameType == Game::ffa) || ( (Game::myGameType == Game::teams) && !onSameTeam );

		if(it->first != ownerPlayerNum && !it->second->hisPlayer->getIsDead() && !it->second->hisPlayer->getIsRespawning() && shouldHome )
		{
			playPos = it->second->hisPlayer->getCenterMass();
			sqDist = (playPos.x-missPos.x)*(playPos.x-missPos.x) + (playPos.y-missPos.y)*(playPos.y-missPos.y);

			if( sqDist < minDist)
			{
				minDist = sqDist;
				targetNum = it->first;
				foundTarget = true;
			}
		}
	}
	
	if (foundTarget)
	{
		sf::Vector2f targetPos =  multiplayer::allPeers.find(targetNum)->second->hisPlayer->getCenterMass();

		accX = targetPos.x - missPos.x;
		accY = targetPos.y - missPos.y;

		float mag = sqrt(velX*velX+velY*velY);

		accX = accX / mag * 5000.f;
		accY = accY / mag * 5000.f;
	}
}

sf::Vector2f missile::getTip()
{
	sf::Vector2f tip;

	sf::ConvexShape trueRect = this->getTrueRect();
	tip = (trueRect.getPoint(1) + trueRect.getPoint(2))*0.5f;

	return tip;

}