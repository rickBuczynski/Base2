#include "messageWindow.h"
#include "Game.h"

messageWindow::messageWindow()
{	
	Game::myConfig.loadFromFile();

	// message
	message = sfg::Label::Create();
	acknowledge = sfg::Button::Create("Close");
	acknowledge->GetSignal( sfg::Widget::OnLeftClick ).Connect( &messageWindow::closeMessage, this);
	sfg::Box::Ptr messageBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );
	messageBox->Pack(message);
	messageBox->Pack(acknowledge);

	window = sfg::Window::Create();
	window->Add(messageBox);
	Game::desktop.Add(window);
	window->Show(false);

	sf::Vector2f windowCenter(  (Game::myConfig.resolution.x -  window->GetClientRect().width )/2 ,
								(Game::myConfig.resolution.y - window->GetClientRect().height)/2);
	window->SetPosition(windowCenter);

}

void messageWindow::closeMessage()
{
	window->Show(false);
}

void messageWindow::displayMessage(std::string _messageText)
{
	std::string messageText = _messageText;
	message->SetText(messageText);
	window->Show(true);
	Game::desktop.BringToFront(window);
}