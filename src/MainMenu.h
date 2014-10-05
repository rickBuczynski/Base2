#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include <SFGUI/SFGUI.hpp>
#include <list>

class MainMenu {
    public:

		MainMenu();

		enum MenuResult { Nothing, Exit, Join, Host, Build };

        // main
        void pickedMultiplayer();
        void pickedExit();
		void pickedBuild();
		void pickedProfile();

		// multiplayer
		void pickedJoin();
		void pickedHost();

		void pickedConnect();

		// back
		void prevPage();

		//profile
		void changedName();
		void OnResSelect();

        MenuResult Show(sf::RenderWindow& renderWindow);

		void restart();

		std::string ip;
 
    private:

		// Integer to determine button actions
		MenuResult menuAction;

		// main
		sfg::Label::Ptr mainTitle;
		sfg::Button::Ptr pickMultiplayer;
		sfg::Button::Ptr pickExit;
		sfg::Button::Ptr pickBuild;
		sfg::Button::Ptr pickProfile;
		sfg::Window::Ptr mainMenuWindow;

		// multiplayer
		sfg::Label::Ptr multiplayerTitle;
		sfg::Button::Ptr pickJoin;
		sfg::Button::Ptr pickHost;
		sfg::Window::Ptr multiplayerWindow;

		// connect
		sfg::Button::Ptr pickConnect;
		sfg::Entry::Ptr enterIp;
		sfg::Window::Ptr connectWindow;
		

		// profile
		sfg::Label::Ptr profileName;
		sfg::Button::Ptr changeName;
		sfg::Entry::Ptr enterName;

		std::map<std::string,sf::Vector2i> resolutions;
		sfg::ComboBox::Ptr resSelect;

		sfg::Window::Ptr profileWindow;
		
		// back
		enum page { main, multiplayer, profile,Connecting };
		page currentPage;

		sfg::Button::Ptr back;
		sfg::Window::Ptr backWindow;
		
};