#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "lobby.h"
#include "Game.h"
#include "multiplayer.h"
#include "lobbyEvent.h"
#include "packet.h"
#include "utils.h"
#include "SHATools.h"

#include <windows.h>
#include <shlobj.h>
#include <dirent.h>

lobby::lobby()
{
	Game::myConfig.loadFromFile();

	playerBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );

	playerWindow = sfg::ScrolledWindow::Create();
	playerWindow->SetScrollbarPolicy( sfg::ScrolledWindow::HORIZONTAL_AUTOMATIC | sfg::ScrolledWindow::VERTICAL_AUTOMATIC );
	playerWindow->AddWithViewport( playerBox );
	playerWindow->SetRequisition( sf::Vector2f( (float)Game::myConfig.resolution.x/2, (float)Game::myConfig.resolution.y - 150.f ) );

	sfg::Box::Ptr actionBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );

	leave = sfg::Button::Create("leave");
	leave->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::pickedLeave, this);
	actionBox->Pack(leave );


	startGame = sfg::Button::Create("start game");
	startGame->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::pickedStartGame, this);
	actionBox->Pack(startGame );

	actionWindow = sfg::Window::Create();
    actionWindow->SetTitle( "RAD Game lobby actions" );
    actionWindow->Add(actionBox);
	actionWindow->SetStyle(0);

	sf::Vector2f windowEdge ( 15.0f ,
							  Game::myConfig.resolution.y - actionWindow->GetClientRect().height  - 25.0f
							);
	actionWindow->SetPosition(windowEdge);

	Game::desktop.Add(playerWindow);
	Game::desktop.Add(actionWindow);

	playerWindow->Show(false);
	actionWindow->Show(false);

	constructGameSetupWindow();
	constructSpecWindows();

	// maps
	mapBox = sfg::Box::Create(sfg::Box::VERTICAL);

	mapWindow = sfg::ScrolledWindow::Create();
	mapWindow->SetScrollbarPolicy( sfg::ScrolledWindow::HORIZONTAL_AUTOMATIC | sfg::ScrolledWindow::VERTICAL_AUTOMATIC );
	mapWindow->AddWithViewport( mapBox );
	mapWindow->SetRequisition( sf::Vector2f( (float)Game::myConfig.resolution.x/2, (float)Game::myConfig.resolution.y - 150.f ) );

	Game::desktop.Add(mapWindow);
	mapWindow->Show(false);

}




void lobby::constructGameSetupWindow()
{
	plusLife = sfg::Button::Create("+");
	plusLife->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::changeGameStatus, this);
	plusLife->SetId("plus life");

	minusLife = sfg::Button::Create("-");
	minusLife->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::changeGameStatus, this);
	minusLife->SetId("minus life");

	curLives = sfg::Label::Create("1");
	lives = sfg::Label::Create("Lives");
	
	sfg::Table::Ptr table( sfg::Table::Create() );
	table->SetRowSpacings(0.5f);

	table->Attach(   lives,     sf::Rect<sf::Uint32>( 1, 0, 1, 1 )   );
	table->Attach(   curLives,  sf::Rect<sf::Uint32>( 1, 1, 1, 1 )   );
	table->Attach(   plusLife,  sf::Rect<sf::Uint32>( 2, 1, 1, 1 )   );
	table->Attach(   minusLife, sf::Rect<sf::Uint32>( 0, 1, 1, 1 )   );

	nextType = sfg::Button::Create(">");
	nextType->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::changeGameStatus, this);
	nextType->SetId("next type");

	prevType = sfg::Button::Create("<");
	prevType->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::changeGameStatus, this);
	prevType->SetId("prev type");

	curType = sfg::Label::Create();
	gameType = sfg::Label::Create("Game Type");

	sfg::Table::Ptr table2( sfg::Table::Create() );
	table2->SetRowSpacings(0.5f);

	table2->Attach(   gameType,     sf::Rect<sf::Uint32>( 1, 0, 1, 1 )   );
	table2->Attach(   curType,  sf::Rect<sf::Uint32>( 1, 1, 1, 1 )   );
	table2->Attach(   nextType,  sf::Rect<sf::Uint32>( 2, 1, 1, 1 )   );
	table2->Attach(   prevType, sf::Rect<sf::Uint32>( 0, 1, 1, 1 )   );

	

	ffDisp = sfg::Label::Create();
	ffText = sfg::Label::Create("Friendly Fire: ");

	ffSel  = sfg::Button::Create();
	ffSel->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::changeGameStatus, this);
	ffSel->SetId("change ff");

	ffBox = sfg::Box::Create( sfg::Box::HORIZONTAL, 0.5f );
	ffBox->Pack(ffText);
	ffBox->Pack(ffDisp);
	ffBox->Pack(ffSel);

	addBot = sfg::Button::Create("Add Bot");
	addBot->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::changeGameStatus, this);
	addBot->SetId("add bot");

	sfg::Box::Ptr aiBox = sfg::Box::Create( sfg::Box::HORIZONTAL, 0.5f );
	aiBox->Pack(addBot);


	setMap = sfg::Button::Create();
	setMap->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::changeGameStatus, this);
	setMap->SetId("set map");

	showMap = sfg::Label::Create();
	sfg::Label::Ptr mapTxt = sfg::Label::Create("Map");

	sfg::Box::Ptr mapBox = sfg::Box::Create( sfg::Box::VERTICAL, 0.5f );
	mapBox->Pack(mapTxt);
	mapBox->Pack(setMap);
	mapBox->Pack(showMap);

	sfg::Box::Ptr setupBox = sfg::Box::Create( sfg::Box::VERTICAL, 25.0f );
	setupBox->Pack(table);
	setupBox->Pack(table2);
	setupBox->Pack(ffBox);
	setupBox->Pack(mapBox);
	setupBox->Pack(aiBox,false,false);

	gameSetupWindow = sfg::Window::Create();
    gameSetupWindow->SetTitle( "Game options" );
	gameSetupWindow->Add(setupBox);

	Game::desktop.Add(gameSetupWindow);

	sf::Vector2f windowEdge ( Game::myConfig.resolution.x -  gameSetupWindow->GetClientRect().width - 50.0f , 15.0f);
	gameSetupWindow->SetPosition(windowEdge);

	gameSetupWindow->SetStyle(sfg::Window::BACKGROUND);

	gameSetupWindow->Show(false);


}

void lobby::constructSpecWindows()
{
	chooseMech = sfg::Button::Create("Choose Mech");
	chooseMech->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::clickedChangeMech, this);


	chooseMechWindow = sfg::Window::Create();
	chooseMechWindow->Add(chooseMech);

	sf::Vector2f windowEdge ( Game::myConfig.resolution.x/2 -  chooseMechWindow->GetClientRect().width/2,
							  Game::myConfig.resolution.y - chooseMechWindow->GetClientRect().height  - 50.0f
							);
	chooseMechWindow->SetPosition(windowEdge);

	chooseMechWindow->SetStyle(0);

	chooseMechWindow->Show(false);

	Game::desktop.Add(chooseMechWindow);

	// load
	loadedBox = sfg::Box::Create(sfg::Box::VERTICAL);

	loadWindow = sfg::ScrolledWindow::Create();
	loadWindow->SetScrollbarPolicy( sfg::ScrolledWindow::HORIZONTAL_AUTOMATIC | sfg::ScrolledWindow::VERTICAL_AUTOMATIC );
	loadWindow->AddWithViewport( loadedBox );
	loadWindow->SetRequisition( sf::Vector2f( (float)Game::myConfig.resolution.x/2, (float)Game::myConfig.resolution.y - 150.f ) );

	Game::desktop.Add(loadWindow);
	loadWindow->Show(false);
}

void lobby::clickedChangeMech()
{
	std::list<sfg::Box::Ptr>::iterator itr = loadedMechs.begin();
	while(itr != loadedMechs.end())
	{
		loadedBox->Remove(*itr); 
		itr++;
	}
	loadedMechs.clear();

	if(loadSpecs())
	{
		loadWindow->Show(true);
		Game::desktop.BringToFront(loadWindow);
	}
	else
	{
		Game::messageWindow.displayMessage("Error loading mech files");
	}

}


void lobby::changeGameStatus()
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );

	const int maxEnum = 1;

	if(widget->GetId() == "plus life")
	{
		if(Game::lives < 255)
			Game::lives++;

		sf::Packet outPacket;
		lobbyEvent livesEvent;
		livesEvent.myType = lobbyEvent::changeLives;
		outPacket << livesEvent;
		multiplayer::sendToClientsTCP(outPacket);
		outPacket.clear();
	}
	else if (widget->GetId() == "minus life")
	{
		if(Game::lives > 1)
			Game::lives--;

		sf::Packet outPacket;
		lobbyEvent livesEvent;
		livesEvent.myType = lobbyEvent::changeLives;
		outPacket << livesEvent;
		multiplayer::sendToClientsTCP(outPacket);
		outPacket.clear();


	}
	else if (widget->GetId() == "next type")
	{	
		int i = (int) Game::myGameType;
		i++;
		if( i > maxEnum)
			i = 0;
		Game::myGameType = (Game::gameType) i;

		sf::Packet outPacket;
		lobbyEvent _event;
		_event.myType = lobbyEvent::changeGameType;
		_event.gameTypeNum = i;
		outPacket << _event;
		multiplayer::sendToClientsTCP(outPacket);
		outPacket.clear();

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
	else if (widget->GetId() == "prev type")
	{
		int i = (int) Game::myGameType;
		i--;
		if( i < 0)
			i = maxEnum;
		Game::myGameType = (Game::gameType) i;

		sf::Packet outPacket;
		lobbyEvent _event;
		_event.myType = lobbyEvent::changeGameType;
		_event.gameTypeNum = i;
		outPacket << _event;
		multiplayer::sendToClientsTCP(outPacket);
		outPacket.clear();

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
	else if (widget->GetId() == "change ff")
	{
		if(Game::ffIsOn)
			Game::ffIsOn = false;
		else
			Game::ffIsOn = true;

		sf::Packet outPacket;
		lobbyEvent _event;
		_event.myType = lobbyEvent::changeFF;
		outPacket << _event;
		multiplayer::sendToClientsTCP(outPacket);
		outPacket.clear();
	}
	else if (widget->GetId() == "add bot")
	{
		multiplayer::addBot();
	}
	else if (widget->GetId() == "set map")
	{

		std::list<sfg::Box::Ptr>::iterator itr = loadedMaps.begin();
		while(itr != loadedMaps.end())
		{
			mapBox->Remove(*itr); 
			itr++;
		}
		loadedMaps.clear();

		//std::cout << "switch map" << std::endl;
		showMaps();
	}


}

void lobby::Show(sf::RenderWindow& renderWindow) 
{
	playerWindow->Show(true);
	actionWindow->Show(true);
	gameSetupWindow->Show(true);
	chooseMechWindow->Show(true);

	std::stringstream _lives;
	_lives << Game::lives;
	curLives->SetText(_lives.str());

	switch (Game::myGameType)
	{
	case Game::ffa:
		curType->SetText("FFA");
		ffBox->Show(false);
		break;
	case Game::teams:
		curType->SetText("teams");
		ffBox->Show(true);
		break;
	}

	setMap->SetLabel(Game::selectedMap);
	showMap->SetText(Game::selectedMap);

	if(multiplayer::_peerState == multiplayer::client)
	{
		startGame->Show(false);
		plusLife->Show(false);
		minusLife->Show(false);
		nextType->Show(false);
		prevType->Show(false);
		addBot->Show(false);

		ffSel->Show(false);
		ffDisp->Show(true);

		setMap->Show(false);
		showMap->Show(true);
	}
	else
	{
		startGame->Show(true);
		plusLife->Show(true);
		minusLife->Show(true);
		nextType->Show(true);
		prevType->Show(true);
		addBot->Show(true);

		ffSel->Show(true);
		ffDisp->Show(false);

		setMap->Show(true);
		showMap->Show(false);
	}


	for(std::map<int,peer*>::iterator it = multiplayer::allPeers.begin(); it != multiplayer::allPeers.end(); ++it)
	{
		std::string buttonArg("Button#");
		sf::Color buttonColor;

		switch(it->second->team)
		{
		case peer::ffa:
			buttonColor = sf::Color::White;
			break;
		case peer::red:
			buttonColor = sf::Color::Red;
			break;
		case peer::green:
			buttonColor = sf::Color::Green;
			break;
		case peer::blue:
			buttonColor = sf::Color::Blue;
			break;
		}

		buttonArg.append(playerIcons.find(it->first)->second->GetId());
		sfg::Context::Get().GetEngine().SetProperty(buttonArg, "BackgroundColor", buttonColor);
		playerIcons.find(it->first)->second->Refresh();
	}

	if(Game::ffIsOn)
	{
		ffSel->SetLabel("On");
		ffDisp->SetText("On");
	}
	else
	{
		ffSel->SetLabel("Off");
		ffDisp->SetText("Off");
	}


	// We're not using SFML to render anything in this program, so reset OpenGL
	// states. Otherwise we wouldn't see anything.
	renderWindow.resetGLStates();

	// Main loop!
	sf::Event event1;

	while(renderWindow.pollEvent( event1 ))
	{

		if( event1.type == sf::Event::Closed ) 
		{
			//std::cout << "closed in lobby" << std::endl;
			multiplayer::quittingFromLobby = true;
			//closeCounter++;
			//std::cout << closeCounter << std::endl;
		}
		else if(event1.type == sf::Event::LostFocus)
			Game::isFocused = false;
		else if(event1.type == sf::Event::GainedFocus)
			Game::isFocused = true;

		Game::desktop.HandleEvent( event1 );

	}

	// Update SFGUI with elapsed seconds since last call.
	Game::desktop.Update( clock.restart().asSeconds() );

	// Rendering.
	renderWindow.clear();
	sfg::Renderer::Get().Display(renderWindow);
	renderWindow.display();

}



void lobby::addIcon(int playerNum, std::string iconText)
{
	sfg::Button::Ptr newIcon = sfg::Button::Create(iconText);
	newIcon->SetId(utils::toString(playerNum));
	newIcon->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::changeTeam, this);

	playerIcons.insert(std::pair<int,sfg::Button::Ptr>(playerNum,newIcon));
	playerBox->Pack(newIcon);
}

void lobby::pickedLeave()
{
	multiplayer::freePeers();
	freeIcons();
	multiplayer::setExitingLobby(true);
	playerWindow->Show(false);
	actionWindow->Show(false);
	gameSetupWindow->Show(false);
	chooseMechWindow->Show(false);
	Game::restartMenu();
}

void lobby::pickedStartGame()
{
	playerWindow->Show(false);
	actionWindow->Show(false);
	gameSetupWindow->Show(false);
	chooseMechWindow->Show(false);
	loadWindow->Show(false);

	char path[ MAX_PATH ];
	SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path );

	std::string mapDir = path;
	mapDir.append("\\Documents\\My Games\\RADgame\\maps\\");
	mapDir.append(Game::selectedMap);
	mapDir.append(".txt");

	std::cout << mapDir << std::endl;

	std::string sha512;
	sha512 = SHATools::getSha512Hash(mapDir, true); 
	std::cout << "The SHA-512 of the map file is: " << sha512 << std::endl;

	sf::Packet outPacket;
	lobbyEvent _event;
	_event.myType = lobbyEvent::launchMap;
	_event.mapHash = sha512;
	outPacket << _event;
	multiplayer::sendToClientsTCP(outPacket);
	outPacket.clear();

	multiplayer::setGameStarted(true);
}

void lobby::freeIcons()
{
	for(std::map<int,sfg::Button::Ptr>::iterator it = playerIcons.begin(); it != playerIcons.end(); ++it)
	{
		playerBox->Remove(it->second);
	}
	playerIcons.clear();

	playerWindow->Show(false);
	actionWindow->Show(false);
	gameSetupWindow->Show(false);
	chooseMechWindow->Show(false);
	loadWindow->Show(false);
}

void lobby::removeIcon(int playerNum)
{
	playerBox->Remove(playerIcons.find(playerNum)->second);
	playerIcons.erase(playerNum);
}

void lobby::leaveLobby()
{
	multiplayer::freePeers();
	freeIcons();
	multiplayer::setExitingLobby(true);
	playerWindow->Show(false);
	actionWindow->Show(false);
	gameSetupWindow->Show(false);
	chooseMechWindow->Show(false);
	loadWindow->Show(false);
	Game::restartMenu();
	Game::mapNames.clear();
}


bool lobby::loadSpecs()
{
	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return false;

	std::string saveDirectory = path;
	saveDirectory.append("\\Documents\\My Games\\RADgame\\saves\\mechs\\");

	DIR *dir;
	struct dirent *ent;
	dir = opendir (saveDirectory.c_str());
	size_t isMech;

	if (dir != NULL) 
	{

		while ((ent = readdir (dir)) != NULL) 
		{
			//printf ("%s\n", ent->d_name);
			std::string fileName = ent->d_name;
			isMech = fileName.find(".txt");
			if (isMech != std::string::npos)
			{
				sfg::Button::Ptr loadedMech = sfg::Button::Create(fileName);
				loadedMech->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::clickedMech, this);
				loadedMech->SetId(fileName);

				// chop off .txt from button
				fileName.erase(fileName.length()-4,fileName.length());
				loadedMech->SetLabel(fileName);

				sfg::Box::Ptr tempBox = sfg::Box::Create();
				tempBox->Pack(loadedMech,false,false);

				loadedBox->Pack(tempBox, false, false);
				loadedMechs.push_back(tempBox);
			}

		}
		closedir (dir);
	} 
	else 
	{
		/* could not open directory */
		return false;
	}

	return true;
}

void lobby::clickedMech()
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );

	std::string mechId = widget->GetId();
	if ( Game::activeSpec.loadFromFile(mechId) )
	{
		multiplayer::allPeers.find(multiplayer::myPlayerNum)->second->spec = Game::activeSpec;
		loadWindow->Show(false);

		sf::Packet outPacket;
		lobbyEvent joinEvent;
		joinEvent.myType = lobbyEvent::changeSpec;
		joinEvent.playerNum = multiplayer::myPlayerNum;
		outPacket << joinEvent;

		if(multiplayer::_peerState == multiplayer::client)
		{
			std::cout << " sending new spec" << std::endl;
			multiplayer::clientLobbySock->send(outPacket);
			outPacket.clear();
		}
		else if(multiplayer::_peerState == multiplayer::server)
		{
			multiplayer::sendToClientsTCP(outPacket);
			outPacket.clear();
		}

	}
	else
	{
		mechId.erase(mechId.length()-4,mechId.length());

		std::string messageText = "Error loading ";
		messageText.append(mechId);
		Game::messageWindow.displayMessage(messageText);
	}

}

void lobby::changeTeam()
{
	if(Game::myGameType == Game::ffa)
		return;

	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );
	std::string id = widget->GetId();

	//std::cout << utils::toInt(id) << std::endl;

	int idNum = utils::toInt(id);

	if( idNum != multiplayer::myPlayerNum && !multiplayer::allPeers.find(idNum)->second->isBot)
		return;

	switch( multiplayer::allPeers.find(idNum)->second->team  )
	{
	case peer::red:
		multiplayer::allPeers.find(idNum)->second->team = peer::green;
		break;
	case peer::green:
		multiplayer::allPeers.find(idNum)->second->team = peer::blue;
		break;
	case peer::blue:
		multiplayer::allPeers.find(idNum)->second->team = peer::red;
		break;
	}

	sf::Packet outPacket;
	lobbyEvent teamChange;

	teamChange.myType = lobbyEvent::changeTeam;
	teamChange.playerNum = idNum;
	teamChange.teamNum = (int) multiplayer::allPeers.find(idNum)->second->team;

	outPacket << teamChange;

	if(multiplayer::_peerState == multiplayer::client)
	{
		multiplayer::clientLobbySock->send(outPacket);
		outPacket.clear();
	}
	else if(multiplayer::_peerState == multiplayer::server)
	{
		multiplayer::sendToClientsTCP(outPacket);
		outPacket.clear();
	}

}

bool lobby::showMaps()
{
	mapWindow->Show(true);
	Game::desktop.BringToFront(mapWindow);

	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return false;

	std::string mapDir = path;
	mapDir.append("\\Documents\\My Games\\RADgame\\maps\\");

	DIR *dir;
	struct dirent *ent;
	dir = opendir (mapDir.c_str());
	size_t isMap;

	if (dir != NULL) 
	{

		while ((ent = readdir (dir)) != NULL) 
		{
			//printf ("%s\n", ent->d_name);
			std::string fileName = ent->d_name;
			isMap = fileName.find(".txt");
			if (isMap != std::string::npos)
			{
				sfg::Button::Ptr loadedMap = sfg::Button::Create(fileName);
				loadedMap->GetSignal( sfg::Widget::OnLeftClick ).Connect( &lobby::clickedMap, this);
				loadedMap->SetId(fileName);

				// chop off .txt from button
				fileName.erase(fileName.length()-4,fileName.length());
				loadedMap->SetLabel(fileName);

				sfg::Box::Ptr tempBox = sfg::Box::Create();
				tempBox->Pack(loadedMap,false,false);

				mapBox->Pack(tempBox, false, false);
				loadedMaps.push_back(tempBox);
			}

		}
		closedir (dir);
	} 
	else 
	{
		/* could not open directory */
		return false;
	}

	return true;
}

void lobby::clickedMap()
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );

	std::string mapName = widget->GetId();
	mapName.erase(mapName.length()-4,mapName.length());

	Game::selectedMap = mapName;

	mapWindow->Show(false);

	sf::Packet outPacket;
	lobbyEvent _event;
	_event.myType = lobbyEvent::changeMap;
	_event.mapName = Game::selectedMap;
	outPacket << _event;
	multiplayer::sendToClientsTCP(outPacket);
	outPacket.clear();

}