#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include <SFGUI/SFGUI.hpp>
#include <list>

class pauseMenu {
    public:

		pauseMenu();

		void quitedToLobby();
		void quitedToMenu();
		void quitedToDesktop();
		

        void setShowing(bool _isShowing);

		sf::Clock clock;

		void showReturnToLobby(bool _isShowing);

    private:

		sfg::Button::Ptr returnToLobby;

		sfg::Button::Ptr quitToMenu;
		sfg::Button::Ptr quitToDesktop;
		sfg::Window::Ptr pauseWindow;



};