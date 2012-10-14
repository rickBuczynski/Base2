#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "MainMenu.h"
#include "Game.h"
#include "multiplayer.h"

MainMenu::MainMenu()
{
	Game::myConfig.loadFromFile();

	// main menu
    mainTitle = sfg::Label::Create( "Main Menu" );

	pickMultiplayer = sfg::Button::Create( "Multiplayer" );
    pickMultiplayer->GetSignal( sfg::Widget::OnLeftClick ).Connect( &MainMenu::pickedMultiplayer, this);
 
	pickExit = sfg::Button::Create( "Exit" );
    pickExit->GetSignal( sfg::Widget::OnLeftClick ).Connect( &MainMenu::pickedExit, this);

	pickBuild = sfg::Button::Create( "Build" );
    pickBuild->GetSignal( sfg::Widget::OnLeftClick ).Connect( &MainMenu::pickedBuild, this);

	pickProfile = sfg::Button::Create("Profile");
	pickProfile->GetSignal( sfg::Widget::OnLeftClick ).Connect( &MainMenu::pickedProfile, this);

	sfg::Box::Ptr mainBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );
    mainBox->Pack( mainTitle );
    mainBox->Pack( pickMultiplayer, false );
	mainBox->Pack( pickBuild, false );
	mainBox->Pack( pickProfile, false );
	mainBox->Pack( pickExit, false );
	

	mainMenuWindow = sfg::Window::Create();
    mainMenuWindow->SetTitle( "RAD Game Test" );
    mainMenuWindow->Add( mainBox );
	mainMenuWindow->SetStyle(0);

	// multiplayer menu
    multiplayerTitle = sfg::Label::Create( "Multiplayer" );

	pickJoin = sfg::Button::Create( "Join" );
    pickJoin->GetSignal( sfg::Widget::OnLeftClick ).Connect( &MainMenu::pickedJoin, this);

	pickHost = sfg::Button::Create( "Host" );
    pickHost->GetSignal( sfg::Widget::OnLeftClick ).Connect( &MainMenu::pickedHost, this);

	sfg::Box::Ptr multiplayerBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );
    multiplayerBox->Pack( multiplayerTitle );
    multiplayerBox->Pack( pickJoin, false );
	multiplayerBox->Pack( pickHost, false );

	multiplayerWindow = sfg::Window::Create();
    multiplayerWindow->SetTitle( "RAD Game multiplayer" );
    multiplayerWindow->Add( multiplayerBox );
	multiplayerWindow->SetStyle(0);

	//connect window
	pickConnect = sfg::Button::Create( "Connect" );
    pickConnect->GetSignal( sfg::Widget::OnLeftClick ).Connect( &MainMenu::pickedConnect, this);

	enterIp = sfg::Entry::Create();

	sfg::Box::Ptr connectBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );
    connectBox->Pack( enterIp, false );
	connectBox->Pack( pickConnect, false );

	connectWindow = sfg::Window::Create();
    connectWindow->Add( connectBox );
	connectWindow->SetStyle(0);
	connectWindow->Show(false);
	Game::desktop.Add(connectWindow);
	connectWindow->SetRequisition(sf::Vector2f(200,1));

	sf::Vector2f conPos(    (Game::myConfig.resolution.x - connectWindow->GetClientRect().width )/2 ,
							(Game::myConfig.resolution.y - connectWindow->GetClientRect().height)/2);
	connectWindow->SetPosition(conPos);

	// profile window
	profileName  = sfg::Label::Create("Profile Name");
	changeName = sfg::Button::Create("Save changes");
	changeName->GetSignal( sfg::Widget::OnLeftClick ).Connect( &MainMenu::changedName, this);
	enterName = sfg::Entry::Create();
	

	resSelect = sfg::ComboBox::Create();
	resSelect->GetSignal( sfg::ComboBox::OnSelect ).Connect( &MainMenu::OnResSelect, this );

	resolutions.insert( std::pair<std::string,sf::Vector2i>("800x600",sf::Vector2i(800,600)) );
	resolutions.insert( std::pair<std::string,sf::Vector2i>("1280x1024",sf::Vector2i(1280,1024)) );
	resolutions.insert( std::pair<std::string,sf::Vector2i>("1920x1080",sf::Vector2i(1920,1080)) );

	// Set the entries of the combo box.
	for(std::map<std::string,sf::Vector2i>::iterator it = resolutions.begin(); it != resolutions.end(); ++it)
	{
		resSelect->AppendItem(it->first);
	}

	sfg::Table::Ptr profTable( sfg::Table::Create() );

	profTable->Attach( profileName, sf::Rect<sf::Uint32>( 0, 0, 1, 1 ), sfg::Table::FILL, sfg::Table::FILL );
	profTable->Attach( enterName, sf::Rect<sf::Uint32>( 0, 1, 1, 1 ) );
	profTable->Attach( changeName, sf::Rect<sf::Uint32>( 0, 2, 1, 1 ) );
	profTable->Attach( resSelect, sf::Rect<sf::Uint32>( 0, 3, 1, 1 ) );

	profileWindow = sfg::Window::Create();
    profileWindow->SetTitle( "RAD Game profile" );
    profileWindow->Add(profTable);
	profileWindow->SetStyle(0);
	

	sf::Vector2f windowCenter(  (Game::myConfig.resolution.x -  mainMenuWindow->GetClientRect().width )/2 ,
								(Game::myConfig.resolution.y - mainMenuWindow->GetClientRect().height)/2);
	mainMenuWindow->SetPosition(windowCenter);


	sf::Vector2f windowCenter2( (Game::myConfig.resolution.x -  multiplayerWindow->GetClientRect().width )/2 ,
								(Game::myConfig.resolution.y - multiplayerWindow->GetClientRect().height)/2);
	multiplayerWindow->SetPosition(windowCenter2);
 
    
	Game::desktop.Add(mainMenuWindow);
	Game::desktop.Add(multiplayerWindow);
	Game::desktop.Add(profileWindow);
	

	// start at main menu
	multiplayerWindow->Show(false);
	profileWindow->Show(false);
	mainMenuWindow->Show(true);
	currentPage = main;

	// back
	back = sfg::Button::Create("Back");
	back->GetSignal( sfg::Widget::OnLeftClick ).Connect( &MainMenu::prevPage, this);
	backWindow = sfg::Window::Create();
	backWindow->Add(back);
	backWindow->SetStyle(0);
	Game::desktop.Add(backWindow);
	backWindow->Show(false);
	sf::Vector2f windowEdge ( 0 , Game::myConfig.resolution.y - backWindow->GetClientRect().height  - 20.0f );
	backWindow->SetPosition(windowEdge);

}
//enum page { multiplayer, host, profile };
void MainMenu::prevPage()
{
	switch(currentPage)
	{
	case multiplayer:
		mainMenuWindow->Show(true);
		multiplayerWindow->Show(false);
		backWindow->Show(false);
		currentPage = main;
		break;
	case profile:
		mainMenuWindow->Show(true);
		profileWindow->Show(false);
		backWindow->Show(false);
		currentPage = main;
		break;
	case Connecting:
		connectWindow->Show(false);
		multiplayerWindow->Show(true);
		currentPage = multiplayer;
		break;
	}
}

void MainMenu::pickedConnect()
{
	std::stringstream field( static_cast<std::string>( enterIp->GetText() ) );
	field >> ip;

	connectWindow->Show(false);
	backWindow->Show(false);
	menuAction = Join;
	Game::loadMapNames();
}

void MainMenu::OnResSelect()
{
	std::stringstream sstr;

	

	config tempConf;
	tempConf.resolution = resolutions.find(static_cast<std::string>( resSelect->GetSelectedText() ) )->second;
	
	if(tempConf.saveToFile())
		Game::messageWindow.displayMessage("Changes will take effect on game restart");
	else
		Game::messageWindow.displayMessage("Error changing resolution");
}


void MainMenu::restart()
{
	multiplayerWindow->Show(false);
	mainMenuWindow->Show(true);
	backWindow->Show(false);
}

void MainMenu::pickedMultiplayer() 
{
	multiplayerWindow->Show(true);
	mainMenuWindow->Show(false);
	backWindow->Show(true);
	currentPage = multiplayer;
}

void MainMenu::pickedExit() 
{
    menuAction = Exit;
	backWindow->Show(false);
}

void MainMenu::pickedBuild() 
{
	mainMenuWindow->Show(false);
	backWindow->Show(false);
    menuAction = Build;
}


void MainMenu::pickedProfile() 
{
	enterName->SetText(Game::myName);
	profileWindow->Show(true);
	mainMenuWindow->Show(false);
	backWindow->Show(true);
	currentPage = profile;
}

void MainMenu::pickedJoin() 
{
	multiplayerWindow->Show(false);
	connectWindow->Show(true);
	currentPage = Connecting;
}

void MainMenu::pickedHost() 
{
	multiplayerWindow->Show(false);
	backWindow->Show(false);
	menuAction = Host;
	Game::loadMapNames();
}

void MainMenu::changedName() 
{
	std::string _name;
	std::stringstream field( static_cast<std::string>( enterName->GetText() ) );
	field >> _name;

	if ( Game::writeName(_name) )
	{
		std::string text = "Changed name to ";
		text.append(_name);
		Game::messageWindow.displayMessage(text);
	}
	else
	{
		Game::messageWindow.displayMessage("Error changing name");
	}
}

MainMenu::MenuResult MainMenu::Show(sf::RenderWindow& renderWindow) 
{
	//Reset menuAction's value to 0
	menuAction = Nothing;
 
    // We're not using SFML to render anything in this program, so reset OpenGL
    // states. Otherwise we wouldn't see anything.
    renderWindow.resetGLStates();


    // Main loop!
    sf::Event event1;
	sf::Clock clock;

	if(Game::isReHosting)
	{
		std::cout << "rehosting" << std::endl;
		pickedMultiplayer();
		pickedHost();
		
	}

	if(Game::isReJoining)
	{
		Game::cleanUp();

		std::cout << "rejoining" << std::endl;
		pickedMultiplayer();
		pickedJoin();
		pickedConnect();

		Game::isReJoining = false;
	}

	while( renderWindow.isOpen() ) 
	{
        // Event processing.
        while( renderWindow.pollEvent( event1 ) )
		{
            Game::desktop.HandleEvent( event1 );
 
            // If window is about to be closed, leave program.
            if( event1.type == sf::Event::Closed ) 
			{
				return Exit;
            }

			else if(event1.type == sf::Event::LostFocus)
				Game::isFocused = false;

			else if(event1.type == sf::Event::GainedFocus)
				Game::isFocused = true;

        }

        // Update SFGUI with elapsed seconds since last call.
        Game::desktop.Update( clock.restart().asSeconds() );
 
        // Rendering.
        renderWindow.clear();
        sfg::Renderer::Get().Display(renderWindow);
        renderWindow.display();

		if(menuAction != Nothing)
			return menuAction;

    }
	return Nothing;

}

