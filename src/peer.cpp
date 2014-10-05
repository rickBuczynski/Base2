#include "peer.h"
#include "Game.h"
#include "networkEvent.h"
#include "packet.h"
#include "utils.h"

peer::peer()
{ 
	path_finder = NULL;
	myTarget = NULL;

	mapReady = false;
}

// used by clients since they don't need to know each peer's address
// they only know the server address
peer::peer(std::string _name, mechSpec _spec)
{ 
	upIsPressed = false;
	downIsPressed = false;
	leftIsPressed = false;
	rightIsPressed = false;

	name = _name;
	spec = _spec;
	std::cout << "adding: " << name << std::endl;

	if(Game::myGameType == Game::teams)
		team = red;
	else
		team = ffa;

	isBot = false;

	path_finder = NULL;
	myTarget = NULL;
	pathingInProgress = false;

	mapReady = false;
}

peer::peer(std::string _name, mechSpec _spec, bool _isBot)
{
	upIsPressed = false;
	downIsPressed = false;
	leftIsPressed = false;
	rightIsPressed = false;

	name = _name;
	spec = _spec;
	std::cout << "adding: " << name << std::endl;

	if(Game::myGameType == Game::teams)
		team = red;
	else
		team = ffa;

	isBot = _isBot;

	if(isBot)
		path_finder = new sf::Thread(&peer::pathFind, this);
	else
		path_finder = NULL;

	myTarget = NULL;
	pathingInProgress = false;

	if(isBot)
		mapReady = true;
	else
		mapReady = false;
}

// used by server it needs to know the address of every peer
peer::peer(std::string _name, mechSpec _spec, sf::IpAddress _address )
{ 
	upIsPressed = false;
	downIsPressed = false;
	leftIsPressed = false;
	rightIsPressed = false;

	name = _name;
	spec = _spec;
	address = _address;
	std::cout << "adding: " << name << " from: " << address << std::endl;

	if(Game::myGameType == Game::teams)
		team = red;
	else
		team = ffa;

	isBot = false;

	path_finder = NULL;
	pathingInProgress = false;

	mapReady = false;
}

peer::~peer()
{
	if(path_finder!=NULL)
	{
		path_finder->terminate();
		delete path_finder;
	}
}

void peer::runAi()
{
	sf::Vector2f target = acquireTarget();

	if(targetAcquired)
	{
		this->hisPlayer->setArmFacing(target);
		sendBotArmAngle();

		if(targetInLos)
		{
			bool armClear = checkLos(this->hisPlayer->getArmTip(),target);
			bool shoulderClear = checkLos(this->hisPlayer->getShoulderCenter(),target);

			Game::handleBotShots(this->hisPlayer,armClear,shoulderClear,target);
		}

		//utils::mutex.lock();
		//setDirection();
		//utils::mutex.unlock();

	}
	
	
	utils::mutex.lock();
	setDirection();
	utils::mutex.unlock();
	
}

bool peer::checkLos(sf::Vector2f initial, sf::Vector2f target)
{
	sf::Vector2f delta = target - initial;
	sf::Vector2f pos = initial;

	utils::normalize(delta);
	delta*=(float)Game::GRID_HEIGHT/2;

	bool passedTarget = false;

	while(!passedTarget)
	{
		if (target.x > initial.x)
			passedTarget = pos.x > target.x;
		else if (target.x < initial.x)
			passedTarget = pos.x < target.x;
		else
			passedTarget = true;

		pos+=delta;

		int i = (int) floor(pos.x / Game::GRID_WIDTH);
		int j = (int) floor(pos.y / Game::GRID_HEIGHT);

		if (i < 0 || i >= Game::mapWidth || j < 0 || j >= Game::mapHeight)
			return false;

		if(Game::gameGrid[i][j].getCollide())
			return false;

	}
	return true;
}

sf::Vector2f peer::acquireTarget()
{

	sf::Vector2f playPos;
	sf::Vector2f myPos = this->hisPlayer->getArmTip();

	float sqDist;

	float minDist;
	float minLosDist;

	int targetNum;
	int targetNumLos;

	bool foundTarget = false;
	bool foundTargetLos = false;

	minDist = (float) Game::mapWidth*Game::GRID_WIDTH*1000;
	minLosDist = minDist;

	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it !=  multiplayer::allPeers.end(); ++it)
	{
		bool onSameTeam = (it->second->team == this->team);
		bool shouldTarget = (Game::myGameType == Game::ffa) || ( (Game::myGameType == Game::teams) && !onSameTeam );

		if(it->first != this->hisPlayer->getPlayerNum() && !it->second->hisPlayer->getIsDead() && !it->second->hisPlayer->getIsRespawning() && shouldTarget )
		{
			playPos = it->second->hisPlayer->getCenterMass();
			sqDist = (playPos.x-myPos.x)*(playPos.x-myPos.x) + (playPos.y-myPos.y)*(playPos.y-myPos.y);

			bool inLos = checkLos(myPos,playPos);

			if( sqDist < minDist)
			{
				minDist = sqDist;
				targetNum = it->first;
				foundTarget = true;
			}
			if( sqDist < minLosDist && inLos)
			{
				minLosDist = sqDist;
				targetNumLos = it->first;
				foundTargetLos = true;
			}
		}
	}
	

	if (foundTargetLos)
	{
		targetAcquired = true;
		targetInLos = true;
		myTarget = multiplayer::allPeers.find(targetNumLos)->second->hisPlayer;
		return multiplayer::allPeers.find(targetNumLos)->second->hisPlayer->getCenterMass();
	}
	else if (foundTarget)
	{
		targetAcquired = true;
		targetInLos = false;
		myTarget = multiplayer::allPeers.find(targetNum)->second->hisPlayer;
		return multiplayer::allPeers.find(targetNum)->second->hisPlayer->getCenterMass();
	}
	else
	{
		targetAcquired = false;
		targetInLos = false;
		myTarget = NULL;
		return sf::Vector2f(0,0);
	}
}

void peer::sendBotArmAngle()
{
	sf::Packet outPacket;

	networkEvent currentNetEvent;
	currentNetEvent.myType = networkEvent::armRotate;
	currentNetEvent.angle = this->hisPlayer->getArmRotation();
	currentNetEvent.playerNumber = this->hisPlayer->getPlayerNum();

	outPacket << currentNetEvent;
	multiplayer::sendToClients(outPacket);
	outPacket.clear();
}

bool boxCmp(box* a, box* b)
{
    return a->fScore() > b->fScore();
}

bool cmpDist(box* a, box* b)
{
	return a->sqDist < b->sqDist;
}

void peer::pathFind()
{
	pathingInProgress = true;

	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it !=  multiplayer::allPeers.end(); ++it)
	{
		if(it->second->isBot && it->first != this->hisPlayer->getPlayerNum() )
		{
			it->second->path_finder->wait();
		}
	}

	if(myTarget == NULL)
	{
		if(this->willFall())
		{
			std::cout << "no tar get back to safety" << std::endl;
			this->getBackToSafety();
		}
	}
	else if(myTarget != NULL)
	{
		if(!multiplayer::allPeers.find(myTarget->getPlayerNum())->second->willFall() )
		{
			/*
			if(!isInLos(myTarget))
			{
				std::cout << "target safe, pursue" << std::endl;
				this->pursuePlayer();
			}
			else if(this->getDistance(myTarget) < 16.0f)
				this->getToDistance();
			else if(this->willFall())
			{
				std::cout << "target safe and in los but i will fall, get to safety" << std::endl;
				this->getBackToSafety();
			}
			*/
			if(this->getDistance(myTarget) < 32.0f)
				this->getToDistance();
			else
				this->pursuePlayer();


		}
		else
		{
			if(this->getDistance(myTarget) < 32.0f)
			{
				this->getToDistance();
			}
			else if(this->willFall())
			{
				std::cout << "target not safe and i will fall, get to safety" << std::endl;
				this->getBackToSafety();
			}

		}
	}

	pathingInProgress = false;
}

void peer::pursuePlayer()
{
	for(int i = 0; i<Game::mapWidth; i++)
	{
		for(int j = 0; j<Game::mapHeight; j++)
		{
			Game::gameGrid[i][j].onClosedList = false;
			Game::gameGrid[i][j].onOpenList = false;
			Game::gameGrid[i][j].shouldOutline = false;
			Game::gameGrid[i][j].parent = NULL;
		}
	}

	int x1 = (int) floor(this->hisPlayer->getBody().GetPosition().x / Game::GRID_WIDTH);
	int y1 = (int) floor(this->hisPlayer->getBody().GetPosition().y / Game::GRID_HEIGHT);
	Game::checkMapBounds(x1,y1);

	int x2 = (int) floor(myTarget->getBody().GetPosition().x  / Game::GRID_WIDTH);
	int y2 = (int) floor(myTarget->getBody().GetPosition().y / Game::GRID_HEIGHT);
	Game::checkMapBounds(x2,y2);

	box* start = &Game::gameGrid[x1][y1];
	box* target = &Game::gameGrid[x2][y2];

	aStar(start,target);
}

void peer::getBackToSafety()
{
	for(int i = 0; i<Game::mapWidth; i++)
	{
		for(int j = 0; j<Game::mapHeight; j++)
		{
			Game::gameGrid[i][j].onClosedList = false;
			Game::gameGrid[i][j].onOpenList = false;
			Game::gameGrid[i][j].shouldOutline = false;
			Game::gameGrid[i][j].parent = NULL;
		}
	}

	int x1 = (int) floor(this->hisPlayer->getBody().GetPosition().x / Game::GRID_WIDTH);
	int y1 = (int) floor(this->hisPlayer->getBody().GetPosition().y / Game::GRID_HEIGHT);
	Game::checkMapBounds(x1,y1);

	box* start = &Game::gameGrid[x1][y1];
	box* target = this->findSafeBox();

	if(target != NULL)
		aStar(start,target);
}

void peer::getToDistance()
{
	std::cout << "too close" << std::endl;

	std::vector<box*> targets;
	std::make_heap(targets.begin(),targets.end(),cmpDist);

	for(int i = 0; i<Game::mapWidth; i++)
	{
		for(int j = 0; j<Game::mapHeight; j++)
		{
			Game::gameGrid[i][j].onClosedList = false;
			Game::gameGrid[i][j].onOpenList = false;
			Game::gameGrid[i][j].shouldOutline = false;
			Game::gameGrid[i][j].parent = NULL;

			if(checkLos(sf::Vector2f((float)(i*Game::GRID_WIDTH),(float)(j*Game::GRID_HEIGHT)),myTarget->getCenterMass()) && Game::gameGrid[i][j].isSafeSpot(this->hisPlayer->clearWidth,this->hisPlayer->clearHeight))
			{
				Game::gameGrid[i][j].sqDist = utils::calcSqDistance(sf::Vector2f((float)(i*Game::GRID_WIDTH),(float)(j*Game::GRID_HEIGHT)),myTarget->getCenterMass());
				targets.push_back(&Game::gameGrid[i][j]);
				std::push_heap(targets.begin(),targets.end(),cmpDist);
			}

		}
	}

	int x1 = (int) floor(this->hisPlayer->getBody().GetPosition().x / Game::GRID_WIDTH);
	int y1 = (int) floor(this->hisPlayer->getBody().GetPosition().y / Game::GRID_HEIGHT);
	Game::checkMapBounds(x1,y1);

	box* start = &Game::gameGrid[x1][y1];
	
	if(!targets.empty())
	{
		box* target = targets.front();
		//target->shouldOutline = true;
		aStar(start,target);
	}
}

void peer::aStar(box* start, box* target)
{
	

	if( target->getHeightAllowed(this->hisPlayer->clearWidth) < this->hisPlayer->clearHeight )
	{
		return;
	}

	sf::Vector2f pathPos;

	std::vector<box*> open;
	std::vector<box*> closed;

	open.insert(open.begin(),start);
	start->onOpenList = true;
	start->gScore = 0;
	start->hScore = 0;

	std::make_heap(open.begin(),open.end(),boxCmp);

	box* current;

	int iters = 0;
	while(true)
	{
		iters++;

		//std::cout << "a" << std::endl;

		// current = element with lowest f
		//std::sort(open.begin(),open.end(),boxCmp);
		

		if(open.size() == 0)
		{
			std::cout << "not found ------------------------------------------ " << std::endl; 
			break;
		}

		current = open.front();
		//current->shouldOutline = true;

		// remove from open list and add to closed
		std::pop_heap(open.begin(),open.end(),boxCmp);
		open.pop_back();

		closed.insert(closed.begin(),current);
		current->onClosedList = true;
		current->onOpenList = false;

		pathPos.x = (float)(current->i_index*Game::GRID_WIDTH );
		pathPos.y =  (float)(current->j_index*Game::GRID_HEIGHT);

		/*
		if(   checkLos( pathPos,targetPlayer->getCenterMass() )   )
		{
			target = current;
			break;
		}
		*/
		

		if(current == target)
		{
			//std::cout << "found path" << std::endl;
			break;
		}
		
		for(int i = current->i_index - 1; i <= current->i_index + 1; i++)
		{
			for(int j = current->j_index - 1; j <= current->j_index + 1; j++)
			{
				Game::checkMapBounds(i,j);

				if	(	!(i==current->i_index && j==current->j_index) 
						&& !Game::gameGrid[i][j].getCollide() 
						&& !Game::gameGrid[i][j].onClosedList
						&& Game::gameGrid[i][j].getHeightAllowed(this->hisPlayer->clearWidth) >= this->hisPlayer->clearHeight
					)
				{
					
					if(Game::gameGrid[i][j].onOpenList == false)
					{
						
						Game::gameGrid[i][j].onOpenList = true;

						Game::gameGrid[i][j].parent = current;

						if(i==current->i_index || j==current->j_index)
							Game::gameGrid[i][j].gScore = current->gScore + 10;// horz or vert
						else
							Game::gameGrid[i][j].gScore = current->gScore + 14;// diag

						Game::gameGrid[i][j].hScore = ( abs(i - target->i_index) + abs(j - target->j_index) )*10;

						open.push_back(&Game::gameGrid[i][j]);
						std::push_heap(open.begin(),open.end(),boxCmp);
					}
					else if(Game::gameGrid[i][j].onOpenList == true)
					{
						int tempG;

						if(i==current->i_index || j==current->j_index)
							tempG = current->gScore + 10;// horz or vert
						else
							tempG = current->gScore + 14;// diag

						if(tempG < Game::gameGrid[i][j].gScore)
						{
							Game::gameGrid[i][j].gScore = tempG;
							Game::gameGrid[i][j].parent = current;
						}
						std::make_heap(open.begin(),open.end(),boxCmp);
					}


				}
			}
		}

	}

	//std::cout << iters << std::endl;

	utils::mutex.lock();
	path.clear();
	while(target->parent != NULL)
	{
		//std::cout << target->i_index << " , " << target->j_index <<std::endl;
		target->shouldOutline = true;
		path.push_front(target);
		target = target->parent;
	}
	utils::mutex.unlock();

	/*
	while(path.size() > 0)
	{
		
		box* curNode =  path.front();
		curNode->shouldOutline = true;
		path.pop_front();
	}
	*/
}

void peer::setDirection()
{
	if(path.size() == 0)
	{
		changeDirection(stopH);
		changeDirection(down);
		return;
	}

	box* next = path.front();
	next->shouldOutline = false;

	sf::Vector2f nextPos;
	nextPos.x = (float)(next->i_index*Game::GRID_WIDTH );
	nextPos.y =  (float)(next->j_index*Game::GRID_HEIGHT);

	sf::FloatRect nextRect;
	nextRect.left = nextPos.x;
	nextRect.top = nextPos.y;
	nextRect.width = (float)Game::GRID_WIDTH;
	nextRect.height = (float)Game::GRID_HEIGHT;

	sf::Vector2f myPos = this->hisPlayer->getBody().GetPosition();

	sf::FloatRect legRect  = this->hisPlayer->GetBoundingRect();
	sf::FloatRect bodyRect = this->hisPlayer->getBody().GetBoundingRect();

	while(  bodyRect.intersects(nextRect)  )
	{
		path.pop_front();

		if(path.size() == 0)
		{
			changeDirection(stopH);
			changeDirection(down);
			return;
		}

		next = path.front();
		next->shouldOutline = false;

		nextPos.x = (float)(next->i_index*Game::GRID_WIDTH );
		nextPos.y =  (float)(next->j_index*Game::GRID_HEIGHT);

		nextRect.left = nextPos.x;
		nextRect.top = nextPos.y;
		nextRect.width = (float)Game::GRID_WIDTH;
		nextRect.height = (float)Game::GRID_HEIGHT;
		//std::cout << "pop and set" << std::endl;
	}

	if(myPos.x < nextPos.x)
	{
		changeDirection(right);
		//std::cout << "right" << std::endl;
	}
	else if(myPos.x > nextPos.x)
	{
		changeDirection(left);
		//std::cout << "left" << std::endl;
	}
	else
	{
		changeDirection(stopH);
	}

	/*
	if(nextPos.y < myPos.y)
	{
		this->upIsPressed = true;
		std::cout << "up" << std::endl;
	}
	else
	{
		this->upIsPressed = false;
	}
	*/

	sf::Vector2f vel = this->hisPlayer->getVel();

	// need to go up
	if(nextPos.y < myPos.y)
	{
		if(vel.y >= 0)
			changeDirection(up);
		else
		{
			float dyNeeded = abs(pathTop() - myPos.y);
			float time = abs(vel.y / this->hisPlayer->gravity);
			float dyPredicted = abs ( (vel.y)*(time) + (0.5f)*(this->hisPlayer->gravity)*(time*time) );

			if (dyPredicted > dyNeeded)
				changeDirection(down);
			else
				changeDirection(up);
		}

	}
	// need to go down
	else
	{
		if(vel.y <= 0)
			changeDirection(down);
		else
		{
			float dyNeeded = abs(pathBottom() - myPos.y);
			float time = abs(vel.y / this->hisPlayer->gravity);
			float dyPredicted = abs ( (vel.y)*(time) + (0.5f)*(this->hisPlayer->gravity)*(time*time) );

			if (dyPredicted > dyNeeded)
				changeDirection(up);
			else
				changeDirection(down);
		}
	}



}

void peer::changeDirection(direction dir)
{
	sf::Packet outPacket;
	networkEvent net;

	

	switch(dir)
	{
	case left:

		if(rightIsPressed)
		{
			net.myType = networkEvent::moveStop;
			net.myMoveKey = networkEvent::right;
			net.playerNumber = this->hisPlayer->getPlayerNum();

			this->rightIsPressed = false;

			outPacket << net;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}

		if(!leftIsPressed)
		{
			net.myType = networkEvent::moveStart;
			net.myMoveKey = networkEvent::left;
			net.playerNumber = this->hisPlayer->getPlayerNum();

			this->leftIsPressed = true;

			outPacket << net;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}

		break;
	case right:

		if(leftIsPressed)
		{
			net.myType = networkEvent::moveStop;
			net.myMoveKey = networkEvent::left;
			net.playerNumber = this->hisPlayer->getPlayerNum();

			this->leftIsPressed = false;

			outPacket << net;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}

		if(!rightIsPressed)
		{
			net.myType = networkEvent::moveStart;
			net.myMoveKey = networkEvent::right;
			net.playerNumber = this->hisPlayer->getPlayerNum();

			this->rightIsPressed = true;

			outPacket << net;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}

		break;
	case up:

		if(!upIsPressed)
		{
			net.myType = networkEvent::moveStart;
			net.myMoveKey = networkEvent::up;
			net.playerNumber = this->hisPlayer->getPlayerNum();

			this->upIsPressed = true;

			outPacket << net;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}


		break;
	case down:

		if(upIsPressed)
		{
			net.myType = networkEvent::moveStop;
			net.myMoveKey = networkEvent::up;
			net.playerNumber = this->hisPlayer->getPlayerNum();

			this->upIsPressed = false;

			outPacket << net;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}

		break;

	case stopH:

		if(leftIsPressed)
		{
			net.myType = networkEvent::moveStop;
			net.myMoveKey = networkEvent::left;
			net.playerNumber = this->hisPlayer->getPlayerNum();

			this->leftIsPressed = false;

			outPacket << net;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}

		if(rightIsPressed)
		{
			net.myType = networkEvent::moveStop;
			net.myMoveKey = networkEvent::right;
			net.playerNumber = this->hisPlayer->getPlayerNum();

			this->rightIsPressed = false;

			outPacket << net;
			multiplayer::sendToClients(outPacket);
			outPacket.clear();
		}

		break;

	}

}


float peer::pathBottom()
{
	std::list<box*>::iterator itr;
	itr = path.begin(); 

	box* prev;
	box* cur;

	prev = *itr;
	itr++;

	while(itr != path.end())
	{
		cur = *itr;

		if(cur == NULL)
			return (float) (prev->j_index*Game::GRID_HEIGHT);
		
		if(cur->j_index <= prev->j_index)
			return (float) (cur->j_index*Game::GRID_HEIGHT);

		prev = cur;

		itr++;
	}

	return (float) (prev->j_index*Game::GRID_HEIGHT);
}


float peer::pathTop()
{
	std::list<box*>::iterator itr;
	itr = path.begin(); 

	box* prev;
	box* cur;

	prev = *itr;
	itr++;

	while(itr != path.end())
	{
		cur = *itr;

		if(cur == NULL)
			return (float) (prev->j_index*Game::GRID_HEIGHT);
		
		if(cur->j_index >= prev->j_index)
			return (float) (cur->j_index*Game::GRID_HEIGHT);

		prev = cur;

		itr++;
	}

	return (float) (prev->j_index*Game::GRID_HEIGHT);
}

box* peer::findSafeBox()
{
	int x = (int) floor(this->hisPlayer->getBody().GetPosition().x / Game::GRID_WIDTH);
	int y = (int) floor(this->hisPlayer->getBody().GetPosition().y / Game::GRID_HEIGHT);
	Game::checkMapBounds(x,y);

	int offSet = 0;
	while( x-offSet >=0 || x+offSet < Game::mapWidth || y-offSet >=0 || y+offSet < Game::mapHeight)
	{
		if(y-offSet >=0 || y+offSet < Game::mapHeight)
		{
			for(int i = std::max(x-offSet,0); i < std::min(x+offSet,Game::mapWidth); i++)
			{
				if(y-offSet>=0)
				{
					if(Game::gameGrid[i][y-offSet].isSafeSpot(this->hisPlayer->clearWidth,this->hisPlayer->clearHeight))
						return &Game::gameGrid[i][y-offSet];
				}

				if(y+offSet < Game::mapHeight)
				{
					if(Game::gameGrid[i][y+offSet].isSafeSpot(this->hisPlayer->clearWidth,this->hisPlayer->clearHeight))
						return &Game::gameGrid[i][y+offSet];
				}
			}
		}

		if(x-offSet >=0 || x+offSet < Game::mapWidth)
		{
			for(int j = std::max(y-offSet,0); j < std::min(y+offSet,Game::mapHeight); j++)
			{
				if( x-offSet >=0)
				{
					if(Game::gameGrid[x-offSet][j].isSafeSpot(this->hisPlayer->clearWidth,this->hisPlayer->clearHeight))
						return &Game::gameGrid[x-offSet][j];
				}

				if( x+offSet < Game::mapWidth)
				{
					if(Game::gameGrid[x+offSet][j].isSafeSpot(this->hisPlayer->clearWidth,this->hisPlayer->clearHeight))
						return &Game::gameGrid[x+offSet][j];
				}
			}
		}


		offSet++;
	}

	return NULL;
}

bool peer::willFall()
{
	sf::FloatRect bodyRect = this->hisPlayer->getBody().GetBoundingRect();
	sf::FloatRect legRect = this->hisPlayer->GetBoundingRect();

	float minX = std::min(bodyRect.left,legRect.left);
	float maxX = std::max(bodyRect.left+bodyRect.width,legRect.left+legRect.width);

	float minY = std::min(bodyRect.top,legRect.top);
	float maxY = std::max(bodyRect.top+bodyRect.height,legRect.top+legRect.height);


	int i_left =   (int) floor(minX / Game::GRID_WIDTH);
	int i_right =  (int) floor(maxX / Game::GRID_WIDTH);
	int j_bottom = (int) floor(maxY / Game::GRID_HEIGHT);

	if (i_left < 0)
		i_left = 0;
	if (i_left >= Game::mapWidth)
		i_left = Game::mapWidth-1;

	if (i_right < 0)
		i_right = 0;
	if (i_right >= Game::mapWidth)
		i_right = Game::mapWidth-1;

	if (j_bottom < 0)
		j_bottom = 0;
	if (j_bottom >= Game::mapHeight)
		j_bottom = Game::mapHeight-1;

	for(int i=i_left; i<i_right; i++)
	{
		for(int j=j_bottom; j<Game::mapHeight; j++)
		{
			if(Game::gameGrid[i][j].getCollide())
				return false;
		}
	}

	return true;
}

bool peer::isInLos(player* tarPlay)
{
	return checkLos( this->hisPlayer->getBody().GetPosition(),tarPlay->getBody().GetPosition() );
}

float peer::getDistance(player* tarPlay)
{
	float sqDist;
	float dx;
	float dy;

	dx = this->hisPlayer->getCenterMass().x - tarPlay->getCenterMass().x;
	dy = this->hisPlayer->getCenterMass().y - tarPlay->getCenterMass().y;

	sqDist = dx*dx + dy*dy;

	float dist = sqrt(sqDist);

	return dist;
}