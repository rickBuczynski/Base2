#pragma once
#include <SFGUI/SFGUI.hpp>

class messageWindow 
{
    public:

		messageWindow();

		void closeMessage();

		void displayMessage(std::string);

    private:

		// message
		sfg::Label::Ptr message;
		sfg::Button::Ptr acknowledge;
		sfg::Window::Ptr window;

};


