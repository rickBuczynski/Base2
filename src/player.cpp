#include "player.h"
#include "Game.h"
#include "box.h"
#include "images.h"
#include "multiplayer.h"
#include "sounds.h"

#include <assert.h>


player::player()
{ 
}

player::player(float x, float y,  int _playerNum, int _lives, mechSpec _spec, sf::Color _color)
{ 

	spec = _spec;
	setParts();
	setStats();

	//start at falling frame falling
	sf::IntRect frameRect   (
						 	0*legStat.frameSize.x,
							0*legStat.frameSize.y,
							legStat.frameSize.x,
							legStat.frameSize.y
							);
	this->GetSprite().setTextureRect(frameRect);


	gravity = 900.f;

	velX = 0;
	velY = 0;
	accX = 0;
	accY = 0;

	playerNum = _playerNum;
	lives = _lives;

	SetPosition((float)Game::myConfig.resolution.x/2, (float)Game::myConfig.resolution.y/2);
	sf::View DefaultView = Game::GetWindow().getDefaultView();

	view = DefaultView;

	GetSprite().move(x-Game::myConfig.resolution.x/2, y-Game::myConfig.resolution.y/2);

	sf::FloatRect legViewRect = this->GetBoundingRect();
	view.setCenter(legViewRect.left, legViewRect.top + legViewRect.height);

	isRespawning = false;
	isDead = false;

	animationFrameX = 0;
	animationFrameY = 0;
	animationTimer = 0;

	stillKnocking = false;

	sf::FloatRect armRect = arm.GetSprite().getLocalBounds();
	arm.GetSprite().setOrigin(0,armRect.top+armRect.height/2);

	sf::FloatRect shoulderRect = shoulder.GetSprite().getLocalBounds();
	shoulder.GetSprite().setOrigin(shoulderRect.left+shoulderRect.width/2,shoulderRect.top+shoulderRect.height/2);

	isLasering = false;
	isCannonCharging = false;

	this->shadeColor(_color);

	clearWidth = (int)ceil( std::max((float)legStat.frameSize.x,body.GetWidth())/(float)Game::GRID_WIDTH  );
	clearHeight = (int)ceil( ( (float)legStat.frameSize.y + body.GetHeight())/(float)Game::GRID_HEIGHT );

	sf::FloatRect legRect = this->GetBoundingRect();
	sf::FloatRect bodyRect = body.GetBoundingRect();

	this->GetSprite().setOrigin(legRect.width/2,legRect.height/2);
	body.GetSprite().setOrigin(bodyRect.width/2,bodyRect.height/2);

	myFire.isShowing = false;
	myFire.curFrame = 0;
	myFire.timer = 0;

	frameRect.left = myFire.curFrame*myFire.fireWidth;
	frameRect.top = 0;
	frameRect.width = myFire.fireWidth;
	frameRect.height = myFire.fireHeight;
	myFire.GetSprite().setTextureRect(frameRect);
	myFire.GetSprite().setOrigin(myFire.fireWidth/2,0);

	this->setPartPositions();
}


player::~player()
{
}

void player::setParts()
{
	loadFromTex(*images::allLegs.find(spec.legName)->second);
	body.loadFromTex(*images::allBodys.find(spec.bodyName)->second);
	arm.loadFromTex(*images::allArms.find(spec.armName)->second);
	shoulder.loadFromTex(*images::allShoulders.find(spec.shoulderName)->second);

	myFire.loadFromTex(images::misc::jetFire);
	myFire.engineSound.setBuffer(*sounds::explosions.find("jetEngine")->second);
	myFire.engineSound.setLoop(true);
}

void player::setStats()
{
	armStat.loadFromFile(spec.armName);
	shoulderStat.loadFromFile(spec.shoulderName);
	legStat.loadFromFile(spec.legName);
	bodyStat.loadFromFile(spec.bodyName);

	maxHealth = bodyStat.health + legStat.health;
	weight = armStat.weight + shoulderStat.weight + legStat.weight + bodyStat.weight;

	maxFuel = bodyStat.fuel;
	jetForce = bodyStat.jetForce;
	
	sheild = 0;

	hasRadar = false;
	for(std::map<int,std::string>::iterator it = spec.equipment.begin(); it !=  spec.equipment.end(); it++)
	{
		std::cout << it->second << std::endl;

		if(it->second == "Radar")
		{
			hasRadar = true;
		}
		else if(it->second == "Sheilding")
		{
			sheild+=0.1f;
		}
		else if(it->second == "Fuel Tank")
		{
			maxFuel+=3.0f;
		}
		else if(it->second == "Improved Hull")
		{
			maxHealth+=56;
		}
		else if(it->second == "Jump Jets")
		{
			jetForce+=400.0f;
		}

	}

	health = maxHealth;
	fuel = maxFuel;

	std::cout << "health: " << health << std::endl;
	std::cout << "weight: " << weight << std::endl;
}


void player::setVel(float x, float y)
{
	velX = x;
	velY = y;
}

void player::setAcc(float x, float y)
{
	accX = x;
	accY = y;
}

void player::pushOut()
{
	sf::FloatRect rect = this->getBodyLegColRect();

	int i_left;
	int j_top;
	int i_right;
	int j_bottom;

	i_left =   (int) floor(rect.left / Game::GRID_WIDTH);
	j_top =    (int) floor(rect.top / Game::GRID_HEIGHT);
	i_right =  (int) floor( (rect.left+rect.width) / Game::GRID_WIDTH);
	j_bottom = (int) floor( (rect.top+rect.height) / Game::GRID_HEIGHT);

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

	int i_center = (i_right+i_left)/2;

	for(int i = i_center; i <= i_right; i++)
	{
		for(int j = j_top; j <= j_bottom; j++)
		{
			if(Game::gameGrid[i][j].getCollide())
			{
				sf::FloatRect boxRect = Game::gameGrid[i][j].GetBoundingRect();
				if(rect.intersects(boxRect))
				{
					//std::cout << "right" << std::endl;
					//std::cout << i << " , " << j << std::endl;
					//std::cout << rect.left << " , " << rect.top << std::endl;
					//std::cout << boxRect.left << " , " << boxRect.top << std::endl;

					this->GetSprite().move(boxRect.left - (rect.left+rect.width),0);
					//this->setPartPositions();

					return;
				}
			}
		}
	}

	for(int i = i_center-1; i >= i_left; i--)
	{
		for(int j = j_top; j <= j_bottom; j++)
		{
			if(Game::gameGrid[i][j].getCollide())
			{
				sf::FloatRect boxRect = Game::gameGrid[i][j].GetBoundingRect();
				if(rect.intersects(boxRect))
				{
					
					//std::cout << "left" << std::endl;
					//std::cout << i << " , " << j << std::endl;
					//std::cout << rect.left << " , " << rect.top << std::endl;
					//std::cout << boxRect.left << " , " << boxRect.top << std::endl;

					this->GetSprite().move(boxRect.left + boxRect.width - rect.left,0);
					//this->setPartPositions();

					return;
				}
			}
		}
	}

}

bool player::checkCollide(float & velX, float & velY, float elapsedTime)
{
	// 6x6 boxes = 36
	//const int max_obj_size = 36;

	sf::FloatRect bodyRect = this->body.GetBoundingRect();
	sf::FloatRect legRect = this->GetBoundingRect();

	bodyRect.left += velX*elapsedTime;
	bodyRect.top += velY*elapsedTime;
	legRect.left += velX*elapsedTime;
	legRect.top += velY*elapsedTime;

	float minX = std::min(bodyRect.left,legRect.left);
	float maxX = std::max(bodyRect.left+bodyRect.width,legRect.left+legRect.width);

	float minY = std::min(bodyRect.top,legRect.top);
	float maxY = std::max(bodyRect.top+bodyRect.height,legRect.top+legRect.height);

	// check collide with other player
	// out for now since it gets stuck when you turn or change frame
	/*
	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it != multiplayer::allPeers.end(); ++it)
	{
		if(  this->isIntersecting( *(it->second->hisPlayer),  elapsedTime,velX,velY) && 
			 it->first != playerNum && !it->second->hisPlayer->getIsRespawning()  )
		{
				velX=0;
				velY=0;
				return true;
		}
	}
	*/

	// check out of screen bounds
	if(  minX < 0 ||
		 maxX > Game::mapWidth*Game::GRID_WIDTH ||
		 minY < 0
	  )
	{
		velX=0;
		velY=0;
		return true;
	}
	else if ( maxY > Game::mapHeight*Game::GRID_HEIGHT )
	{
		velX=0;
		velY=0;

		// hit bottom = death
		lives-=1;
		health=maxHealth;

		respawnTimer.restart();
		isRespawning=true;

		if(lives <= 0)
		{
			std::cout << "dead" << std::endl;
			isDead = true;
			isKnocked = false;
			stillKnocking = false;
		}

		return true;
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

	for(int i = i_left; i <= i_right; i++)
	{
		for(int j = j_top; j <= j_bottom; j++)
		{
			// not using optimization now but can be put back in if mech shapes are more normal
			// outer if is performance optimization
			// only check outside of object not inside grid
			//if(i == i_left || i == i_right || j == j_top || j == j_bottom)
			//{

				if(Game::gameGrid[i][j].getCollide())
				{
					if( this->isIntersecting(Game::gameGrid[i][j] ,elapsedTime,velX,velY) )
					{
						velX=0;
						velY=0;

						//std::cout << "col" << std::endl;

						if(Game::gameGrid[i][j].GetPosition().y >= this->getBodyLegColRect().top+this->getBodyLegColRect().height)
							isGrounded = true;

						return true;
					}
				}
			//}
		}
	}

	
	return false;
}

void player::Update(float elapsedTime)
{

	float timeDelta = respawnTimer.getElapsedTime().asSeconds();
	if( timeDelta > 4.0f && isRespawning == true && isDead == false)
	{
		GetSprite().setPosition((float)(playerNum+5)*4.f*Game::GRID_WIDTH,32*Game::GRID_HEIGHT);

		if(playerNum==multiplayer::myPlayerNum)
		{
			sf::FloatRect legViewRect = this->GetBoundingRect();
			view.setCenter(legViewRect.left, legViewRect.top + legViewRect.height);
		}
		isRespawning = false;
		respawnTimer.restart();
	}
	
	if(isKnocked)
	{
		if ( knockTimer.getElapsedTime() > knockDuration )
		{	
			knockTimer.restart();
			isKnocked = false;
		}
	}

	if(isGrounded)
	{
		knockTimer.restart();
		isKnocked = false;
		stillKnocking = false;
	}
	
	velY += accY * elapsedTime;
	velX += accX * elapsedTime;

	sf::Vector2f pos = this->GetPosition();

	bool goingUp;
	bool goingLeft;
	bool goingRight;

	if(playerNum == multiplayer::myPlayerNum)
	{
		goingUp = sf::Keyboard::isKeyPressed(sf::Keyboard::W) && Game::isFocused;
		goingLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::A) && Game::isFocused;
		goingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::D) && Game::isFocused;
	}
	else
	{

		goingUp = multiplayer::allPeers.find(playerNum)->second->upIsPressed;
		goingLeft = multiplayer::allPeers.find(playerNum)->second->leftIsPressed;
		goingRight = multiplayer::allPeers.find(playerNum)->second->rightIsPressed;
	}

	if ( (!isKnocked && (goingUp || goingLeft || goingRight) ) )
		stillKnocking = false;

	if( isRespawning || isDead )
	{
		stillKnocking = false;
		isKnocked = false;
	}

	float speed = legStat.moveSpeed/weight*400.f;

	if(isKnocked || stillKnocking)
		velX = knockDirection.x;
	else if(goingLeft && !isRespawning)
		velX = -1*speed;
	else if(goingRight && !isRespawning)		
		velX = speed;
	else
		velX= 0;

	if(goingUp && !isRespawning && !isKnocked && fuel > 0)
	{
		fuel -= elapsedTime;
		accY = -1*jetForce;
	}
	else if (!isRespawning) 
	{
		accY = gravity;
	}
	else
	{
		fuel = maxFuel;
		accY = 0;
		velY = 0;
	}

	// dummies allow checkCollide to only set x to 0 if moving causes an x collsion
	// and y to 0 if moving causes a y collision
	float dummyVelX = 0;
	float dummyVelY = 0;
	isGrounded = false;
	this->checkCollide(velX, dummyVelY, elapsedTime);
	bool isYCollision = this->checkCollide(dummyVelX, velY, elapsedTime);

	setAnimationFrame(elapsedTime);
	this->pushOut();
	//sf::FloatRect rect = this->getBodyLegColRect();
	//std::cout << rect.left << " , " << rect.top << std::endl;

	GetSprite().move(velX * elapsedTime, velY * elapsedTime);
	sf::FloatRect legViewRect = this->GetBoundingRect();
	view.setCenter(legViewRect.left, legViewRect.top + legViewRect.height);

	sf::Vector2f viewSize = view.getSize();
	sf::Vector2f viewPos = view.getCenter();

	if(viewPos.x - viewSize.x/2 < 0 )
		viewPos.x = viewSize.x/2;

	if(viewPos.y - viewSize.y/2 < 0 )
		viewPos.y = viewSize.y/2;

	if(viewPos.x + viewSize.x/2 > Game::mapWidth*Game::GRID_WIDTH )
		viewPos.x = Game::mapWidth*Game::GRID_WIDTH - viewSize.x/2;

	if(viewPos.y + viewSize.y/2 > Game::mapHeight*Game::GRID_HEIGHT  )
		viewPos.y = Game::mapHeight*Game::GRID_HEIGHT - viewSize.y/2;

	view.setCenter(viewPos);

	//std::cout << view.getCenter().x << " , " << view.getCenter().y << std::endl;

	setPartPositions();

	if(isGrounded)
		fuel += elapsedTime;

	if(fuel > maxFuel)
		fuel = maxFuel;
	else if(fuel < 0)
		fuel = 0;

}

sf::View player::getView()
{
	return view;
}


int player::getPlayerNum()
{
	return playerNum;
}

void player::takeDamage(int weaponDamage)
{
	health-= (int) ceil( (float)weaponDamage * (1-sheild) );
	if(health <= 0)
	{
		lives-=1;
		health=maxHealth;

		respawnTimer.restart();
		isRespawning=true;

	}
	if(lives <= 0)
	{
		std::cout << "dead" << std::endl;
		isDead = true;
		isKnocked = false;
		stillKnocking = false;
	}
	//else
		//std::cout << playerNum << ": " << health << "hp || " << lives << " lives" << std::endl;
}


void player::kill()
{
	isRespawning=true;
	isDead = true;
	isKnocked = false;
	stillKnocking = false;
}


void player::syncPosition(float x, float y)
{
	GetSprite().move(x-this->GetPosition().x,y-this->GetPosition().y);

	if(playerNum == multiplayer::myPlayerNum)
	{
		sf::FloatRect legViewRect = this->GetBoundingRect();
		view.setCenter(legViewRect.left, legViewRect.top + legViewRect.height);
	}

	sf::Vector2f viewSize = view.getSize();
	sf::Vector2f viewPos = view.getCenter();

	if(viewPos.x - viewSize.x/2 < 0 )
		viewPos.x = viewSize.x/2;

	if(viewPos.y - viewSize.y/2 < 0 )
		viewPos.y = viewSize.y/2;

	if(viewPos.x + viewSize.x/2 > Game::mapWidth*Game::GRID_WIDTH )
		viewPos.x = Game::mapWidth*Game::GRID_WIDTH - viewSize.x/2;

	if(viewPos.y + viewSize.y/2 > Game::mapHeight*Game::GRID_HEIGHT  )
		viewPos.y = Game::mapHeight*Game::GRID_HEIGHT - viewSize.y/2;

	view.setCenter(viewPos);
}

sf::Vector2f player::getVel()
{
	sf::Vector2f vel;
	vel.x = velX;
	vel.y = velY;

	return vel;
}

void player::Draw(sf::RenderWindow & renderWindow)
{
	if( IsLoaded()  && !isRespawning )
	{

		if(myFire.isShowing)
			renderWindow.draw(myFire.GetSprite());

		// draw legs
		renderWindow.draw(GetSprite());
		
		// draw other parts
		renderWindow.draw(body.GetSprite());

		renderWindow.draw(shoulder.GetSprite());

		renderWindow.draw(arm.GetSprite());
		/*
		sf::FloatRect colRect = this->getBodyLegColRect();
		sf::RectangleShape showCol;
		showCol.setOutlineColor(sf::Color::Red);
		showCol.setOutlineThickness(2);
		showCol.setFillColor(sf::Color::Transparent);
		showCol.setSize(sf::Vector2f(colRect.width,colRect.height));
		showCol.setPosition(sf::Vector2f(colRect.left,colRect.top));

		renderWindow.draw(showCol);
		*/
	}

	/*

	sf::Vector2f end = this->getArmTip();
	sf::Vector2f start = arm.GetSprite().getPosition();

	sf::Vector2f points[5];

	sf::Vector2f mid = (end+start)*0.5f;

	points[0] = start;
	points[1] = (start+mid)*0.5f;
	points[2] = mid;
	points[3] = (mid+end)*0.5f;
	points[4] = end;

	sf::CircleShape cir;
	cir.setRadius(3.0f);
	cir.setOrigin(cir.getRadius()/2,cir.getRadius()/2);
	cir.setFillColor(sf::Color::Red);


	for(int i=0; i<5; i++)
	{
		cir.setPosition(points[i]);
		renderWindow.draw(cir);
	}

	*/

}

void player::setPartPositions()
{
	
	
	float angle = this->getArmRotation();
	const float pi = 3.14159265f;

	

	sf::FloatRect legRect = this->GetBoundingRect();

	// position body
	body.SetPosition( legRect.left - bodyStat.legOffset + body.GetWidth()/2 , legRect.top-body.GetHeight()/2 );
	sf::Vector2f off;

	off = legStat.frameOffsets[animationFrameX][animationFrameY];

	if(off.x > 0 || off.y > 0)
	{
		off.x -= legStat.frameSize.x * (float)animationFrameX;
		off.y -= legStat.frameSize.y * (float)animationFrameY;
		//std::cout << off.x << " , " << off.y << std::endl;

		if(cos(angle*pi/180) < 0 )
		{
			body.GetSprite().move(off);
		}
		else
		{
			body.GetSprite().move(0,off.y);
			body.GetSprite().move(this->GetWidth()-off.x-legStat.waistSize,0);
		}
	}

	sf::FloatRect bodyRect = body.GetBoundingRect();
	
	

	sf::FloatRect shoulderRect = shoulder.GetBoundingRect();

	//shoulder.SetPosition(   bodyRect.left + bodyRect.width - bodyStat.shoulderPoint.x , 
	//							bodyRect.top + bodyStat.shoulderPoint.x);

	
	if(cos(angle*pi/180) < 0)
	{
		this->scale(1.0f,1.0f);
		body.scale(-1.0f,1.0f);

		arm.SetPosition( bodyRect.left + bodyRect.width - bodyStat.armPoint.x , bodyRect.top + bodyStat.armPoint.y );
		arm.scale(1.0f,-1.0f);

		myFire.SetPosition( bodyRect.left + bodyRect.width - bodyStat.firePoint.x , bodyRect.top + bodyStat.firePoint.y );

		shoulder.SetPosition(   bodyRect.left + bodyRect.width - bodyStat.shoulderPoint.x , 
								bodyRect.top + bodyStat.shoulderPoint.y);
	}
	else
	{
		this->scale(-1.0f,1.0f);
		body.scale(1.0f,1.0f);

		arm.SetPosition( bodyRect.left + bodyStat.armPoint.x , bodyRect.top + bodyStat.armPoint.y );
		arm.scale(1.0f,1.0f);

		myFire.SetPosition( bodyRect.left + bodyStat.firePoint.x , bodyRect.top + bodyStat.firePoint.y );

		shoulder.SetPosition(   bodyRect.left + bodyStat.shoulderPoint.x ,
								bodyRect.top + bodyStat.shoulderPoint.y);
	}
	
	

	if(this->playerNum == multiplayer::myPlayerNum)
	{
		sf::Vector2f woldPos;
		woldPos = Game::GetWindow().convertCoords( sf::Mouse::getPosition(Game::GetWindow()) );
		arm.rotateToFace(woldPos.x, woldPos.y);
	}
}

bool player::getIsRespawning()
{
	return isRespawning;
}

bool player::getIsDead()
{
	return isDead;
}

void player::setAnimationFrame(float elapsedTime)
{
	sf::IntRect frameRect;

	bool goingUp;
	bool goingLeft;
	bool goingRight;

	if(playerNum == multiplayer::myPlayerNum)
	{
		goingUp = sf::Keyboard::isKeyPressed(sf::Keyboard::W) && Game::isFocused && !isRespawning;
		goingLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::A) && Game::isFocused && !isRespawning;
		goingRight = sf::Keyboard::isKeyPressed(sf::Keyboard::D) && Game::isFocused && !isRespawning;
	}
	else
	{

		goingUp = multiplayer::allPeers.find(playerNum)->second->upIsPressed && !isRespawning;
		goingLeft = multiplayer::allPeers.find(playerNum)->second->leftIsPressed && !isRespawning;
		goingRight = multiplayer::allPeers.find(playerNum)->second->rightIsPressed && !isRespawning;
	}

	myFire.isShowing = false;
	if(goingUp)
	{
		animationFrameX= 0;
		animationFrameY= 1;
		animationTimer= 0;

		myFire.timer+=elapsedTime;
		//std::cout << myFire.timer << std::endl;
		//std::cout << myFire.timer << std::endl;
		if(myFire.timer > (float)(myFire.msPerFrame)*0.001f)
		{
			myFire.timer = 0;
			myFire.curFrame++;
			//std::cout << "+1" << std::endl;
		}
		myFire.curFrame = myFire.curFrame % myFire.totFrames;

		myFire.isShowing = true;

		frameRect.left = myFire.curFrame*myFire.fireWidth;
		frameRect.top = 0;
		frameRect.width = myFire.fireWidth;
		frameRect.height = myFire.fireHeight;

		//std::cout << myFire.timer << std::endl;
		myFire.GetSprite().setTextureRect(frameRect);

		if(myFire.engineSound.getStatus() != sf::Sound::Playing)
			myFire.engineSound.play();
		
	}
	else if(goingLeft && isGrounded)
	{
		animationFrameY = 0;
		animationTimer+=elapsedTime*abs(velX);
		if(animationTimer > (float)legStat.pixPerFrame)
		{
			animationTimer = 0;
			animationFrameX++;
		}
		animationFrameX = animationFrameX % legStat.xFrames;
		myFire.engineSound.pause();
	}
	else if(goingRight && isGrounded)		
	{
		animationFrameY = 0;
		animationTimer+=elapsedTime*abs(velX);
		if(animationTimer > (float)legStat.pixPerFrame)
		{
			animationTimer = 0;
			animationFrameX++;
		}
		animationFrameX = animationFrameX % legStat.xFrames;
		myFire.engineSound.pause();
	}
	else
	{
		animationFrameX= 0;
		animationFrameY= 1;
		animationTimer= 0;
		myFire.engineSound.pause();
	}
	
	frameRect.left = animationFrameX*legStat.frameSize.x;
	frameRect.top =	animationFrameY*legStat.frameSize.y;
	frameRect.width = legStat.frameSize.x;
	frameRect.height = legStat.frameSize.y;					
	this->GetSprite().setTextureRect(frameRect);

}


sf::Vector2f player::getArmTip()
{
	sf::Vector2f armTip;

	sf::ConvexShape armRect = arm.getTrueRect();
	//float angle = arm.getRotation();

	//const float pi = 3.14159265f;
	//angle = angle*pi/180;

	//armTip.x = armRect.left+armRect.width*cos(angle);
	//armTip.y = armRect.top+armRect.width*sin(angle)-armRect.height/2;

	armTip.x = ( armRect.getPoint(1).x + armRect.getPoint(2).x )/ 2;
	armTip.y = ( armRect.getPoint(1).y + armRect.getPoint(2).y )/ 2;

	return armTip;
}


sf::Vector2f player::getShoulderCenter()
{
	sf::Vector2f shoulderCenter;

	sf::FloatRect shoulderRect = shoulder.GetBoundingRect();

	shoulderCenter.x = shoulderRect.left+shoulderRect.width/2;
	shoulderCenter.y = shoulderRect.top+shoulderRect.height/2;

	return shoulderCenter;
}

float player::getArmRotation() const
{
	return arm.getRotation();
}

void player::setArmRotation(float angle)
{
	arm.setRotation(angle);
}

void player::setArmFacing(sf::Vector2f pos)
{
	arm.rotateToFace(pos.x,pos.y);
}

int player::getHealth()
{
	return health;
}

int player::getLives()
{
	return lives;
}

bool player::isIntersecting(const VisibleGameObject & vgo, float frameTime, float velX, float velY)
{
	sf::FloatRect legRect = this->getColRect();
	sf::FloatRect bodyRect = body.GetBoundingRect();

	legRect.left+=velX*frameTime;
	legRect.top+=velY*frameTime;

	bodyRect.left+=velX*frameTime;
	bodyRect.top+=velY*frameTime;


	sf::FloatRect colRect = vgo.GetBoundingRect();


	if( legRect.intersects(colRect) ||
		bodyRect.intersects(colRect) 	)
	{
		return true;
	}
	else
	{
		return false;
	}
}


bool player::isIntersecting(const box & b, float frameTime, float velX, float velY)
{
	sf::FloatRect plyRect = this->getBodyLegColRect();

	plyRect.left+=velX*frameTime;
	plyRect.top+=velY*frameTime;

	sf::FloatRect objRect = b.GetBoundingRect();

	if( plyRect.intersects(objRect) )
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool player::isIntersecting(const player & _player, float frameTime, float velX, float velY)
{
	sf::FloatRect myRect = this->getBodyLegColRect();

	myRect.left+=velX*frameTime;
	myRect.top+=velY*frameTime;

	sf::FloatRect hisRect = _player.getBodyLegColRect();

	if( myRect.intersects(hisRect) )
	{
		return true;
	}
	else
	{
		return false;
	}

}

bool player::containsPoint(sf::Vector2f pos)
{
	sf::FloatRect legRect = this->getColRect();
	sf::FloatRect bodyRect = body.GetBoundingRect();

	if( legRect.contains(pos) || bodyRect.contains(pos) )
		return true;
	else
		return false;
}

void player::knockBack(sf::Vector2f direction,sf::Time duration)
{
	knockDirection = direction;
	knockDuration = duration;
	isKnocked = true;
	stillKnocking = true;
	isGrounded = false;
	knockTimer.restart();

	velX = knockDirection.x;
	velY = knockDirection.y;
}

const VisibleGameObject & player::getBody() const
{
	return this->body;
}

const VisibleGameObject & player::getArm() const
{
	return this->arm;
}

const VisibleGameObject & player::getShoulder() const
{
	return this->shoulder;
}

armStat player::getArmStat()
{
	return this->armStat;
}

shoulderStat player::getShoulderStat()
{
	return this->shoulderStat;
}

sf::Vector2f player::getCenterMass()
{
	sf::FloatRect legRect = GetBoundingRect();
	sf::Vector2f com(legRect.left+legRect.width/2,legRect.top);
	return com;
}

void player::shadeColor(sf::Color color)
{
		GetSprite().setColor(color);
		body.GetSprite().setColor(color);
		shoulder.GetSprite().setColor(color);
		arm.GetSprite().setColor(color);
}

float player::getFuel()
{
	return fuel;
}

sf::FloatRect player::getColRect() const
{
	float angle = this->getArmRotation();
	const float pi = 3.14159265f;

	sf::FloatRect colRect = this->GetBoundingRect();

	sf::Vector2f off;

	off = legStat.frameOffsets[animationFrameX][animationFrameY];

	if(off.x > 0 || off.y > 0)
	{
		off.x -= legStat.frameSize.x * (float)animationFrameX;
		off.y -= legStat.frameSize.y * (float)animationFrameY;
		//std::cout << off.x << " , " << off.y << std::endl;

		if(cos(angle*pi/180) < 0 )
		{
			colRect.left+=off.x;
			colRect.width-=off.x;

			colRect.left-=legStat.waistToEdgeDist;
			colRect.width+=legStat.waistToEdgeDist;


			//body.GetSprite().move(off);
		}
		else
		{
			colRect.width-=off.x;

			colRect.width+=legStat.waistToEdgeDist;

			//body.GetSprite().move(0,off.y);
			//body.GetSprite().move(this->GetWidth()-off.x-legStat.waistSize,0);
		}
	}

	return colRect;



}

sf::FloatRect player::getBodyLegColRect() const
{
	sf::FloatRect legRect = this->getColRect();
	sf::FloatRect bodyRect = body.GetBoundingRect();

	float minX;
	float maxX;
	float minY;
	float maxY;

	minX = std::min(legRect.left,bodyRect.left);
	maxX = std::max(legRect.left+legRect.width,bodyRect.left+bodyRect.width);

	minY = std::min(legRect.top,bodyRect.top);
	maxY = std::max(legRect.top+legRect.height,bodyRect.top+bodyRect.height);

	sf::FloatRect rect;

	rect.left = minX;
	rect.top = minY;

	rect.width = maxX - minX;
	rect.height = maxY - minY;

	return rect;


}


bool player::armColliding()
{
	sf::FloatRect armRect = this->arm.GetBoundingRect();

	sf::Vector2f end = this->getArmTip();
	sf::Vector2f start = arm.GetSprite().getPosition();

	sf::Vector2f points[5];

	sf::Vector2f mid = (end+start)*0.5f;

	points[0] = start;
	points[1] = (start+mid)*0.5f;
	points[2] = mid;
	points[3] = (mid+end)*0.5f;
	points[4] = end;

	// check out of screen bounds
	if(  armRect.left < 0 ||
		 armRect.left+armRect.width > Game::mapWidth*Game::GRID_WIDTH ||
		 armRect.top < 0 ||
		 armRect.top+armRect.height > Game::mapHeight*Game::GRID_HEIGHT 
	  )
	{
		return true;
	}

	int i_left;
	int j_top;
	int i_right;
	int j_bottom;

	i_left =   (int) floor(armRect.left / Game::GRID_WIDTH);
	j_top =    (int) floor(armRect.top / Game::GRID_HEIGHT);
	i_right =  (int) floor( (armRect.left+armRect.width)  / Game::GRID_WIDTH);
	j_bottom = (int) floor( (armRect.top+armRect.height)  / Game::GRID_HEIGHT);

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

	for(int i = i_left; i <= i_right; i++)
	{
		for(int j = j_top; j <= j_bottom; j++)
		{


			if(Game::gameGrid[i][j].getCollide())
			{
				//std::cout << i << " , " << j << std::endl;
				for(int k=0; k<5; k++)
				{
					if( Game::gameGrid[i][j].GetBoundingRect().contains(points[k]) )
					{
						return true;
					}
				}
			}


		}
	}

	
	return false;

}