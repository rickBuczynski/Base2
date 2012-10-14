#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include "pauseMenu.h"
#include "Game.h"
#include "multiplayer.h"

pauseMenu::pauseMenu()
{
	Game::myConfig.loadFromFile();

	quitToMenu = sfg::Button::Create( "Quit to Menu" );
    quitToMenu->GetSignal( sfg::Widget::OnLeftClick ).Connect( &pauseMenu::quitedToMenu, this);
 
	quitToDesktop = sfg::Button::Create( "Quit to Desktop" );
    quitToDesktop->GetSignal( sfg::Widget::OnLeftClick ).Connect( &pauseMenu::quitedToDesktop, this);

	returnToLobby = sfg::Button::Create("Return to Lobby");
	returnToLobby->GetSignal( sfg::Widget::OnLeftClick ).Connect( &pauseMenu::quitedToLobby, this);

	sfg::Box::Ptr mainBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );
	mainBox->Pack( returnToLobby, false );
    mainBox->Pack( quitToMenu, false );
	mainBox->Pack( quitToDesktop, false );

	pauseWindow = sfg::Window::Create();
    pauseWindow->SetTitle( "Pause menu" );
    pauseWindow->Add( mainBox );
    
	Game::desktop.Add(pauseWindow);

	pauseWindow->Show(false);
	returnToLobby->Show(false);

}


void pauseMenu::quitedToMenu() 
{
	pauseWindow->Show(false);
	// false means not a server DC
	Game::quitToMenu(false);
}

void pauseMenu::quitedToDesktop() 
{
	pauseWindow->Show(false);
	Game::quitToDesktop();
}

void pauseMenu::quitedToLobby()
{
	pauseWindow->Show(false);
	Game::quitToLobby(false);
}

void pauseMenu::setShowing(bool _isShowing)
{
	pauseWindow->Show(_isShowing);

	if(multiplayer::_peerState == multiplayer::server && _isShowing)
		showReturnToLobby(true);
	else
		showReturnToLobby(false);

}

void pauseMenu::showReturnToLobby(bool _isShowing)
{
	returnToLobby->Show(_isShowing);
}