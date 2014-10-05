#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include <SFGUI/SFGUI.hpp>
#include <list>

#include "multiplayer.h"

class lobby 
{
    public:

		lobby();

		void Show(sf::RenderWindow& renderWindow);

		void setIconShow(int playerNum, std::string iconText);

		void addIcon(int playerNum, std::string iconText);
 
		void pickedLeave();
		void pickedStartGame();

		void freeIcons();

		void leaveLobby();

		void removeIcon(int playerNum);

		void changeGameStatus();

		void clickedChangeMech();
		void clickedMech();


		bool loadSpecs();

		void changeTeam();

		bool showMaps();
		void clickedMap();

    private:
		sf::Clock clock;

		void constructGameSetupWindow();
		void constructSpecWindows();

		std::map<int,sfg::Button::Ptr> playerIcons;

		sfg::Button::Ptr leave;
		sfg::Button::Ptr startGame;

		sfg::Box::Ptr playerBox;

		sfg::ScrolledWindow::Ptr playerWindow;
		sfg::Window::Ptr actionWindow;


		sfg::Window::Ptr gameSetupWindow;

		sfg::Button::Ptr plusLife;
		sfg::Button::Ptr minusLife;
		sfg::Label::Ptr lives;
		sfg::Label::Ptr curLives;

		sfg::Button::Ptr nextType;
		sfg::Button::Ptr prevType;
		sfg::Label::Ptr gameType;
		sfg::Label::Ptr curType;

		sfg::Button::Ptr setMap;
		sfg::Label::Ptr showMap;

		sfg::Button::Ptr addBot;

		// friendly fire
		sfg::Box::Ptr ffBox;
		sfg::Button::Ptr ffSel;
		sfg::Label::Ptr ffDisp;
		sfg::Label::Ptr ffText;
		
		sfg::Window::Ptr chooseMechWindow;
		sfg::Button::Ptr chooseMech;

		// load
		std::list<sfg::Box::Ptr> loadedMechs;
		sfg::Box::Ptr loadedBox;
		sfg::ScrolledWindow::Ptr loadWindow;


		//maps
		std::list<sfg::Box::Ptr> loadedMaps;
		sfg::Box::Ptr mapBox;
		sfg::ScrolledWindow::Ptr mapWindow;

};