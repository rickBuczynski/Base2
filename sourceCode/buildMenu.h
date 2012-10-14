#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include <SFGUI/SFGUI.hpp>
#include <list>

#include "mechSpec.h"

class buildMenu {
    public:

		buildMenu();

		// main
		void newOrLoad();
		void exitBuildMenu();

		// change
		void changeLegs();
		void changeArms();
		void changeBodys();
		void changeShoulders();

		void clickedSave();
		void backedToMain();

		// load
		void clickedMech();

		// delete
		void clickedMechDelete();

		// part select
		void selectLeg();
		void selectBody();
		void selectArm();
		void selectShoulder();
		void doneChanging();

		// back
		void prevPage();

		// equipment
		void changeEquip();
		void selectEquip();

		enum result { stillBuilding, quit, done };
		result menuResult;

		void buildMenu::start();

		result Show(sf::RenderWindow& renderWindow);

		bool loadSpecs();
		bool loadSpecsDelete();

    private:

		

		mechSpec workingSpec;
		mechSpec tempSpec;

		// main
		sfg::Button::Ptr newSpec;
		sfg::Button::Ptr loadSpec;
		sfg::Button::Ptr deleteSpec;
		sfg::Window::Ptr mainWindow;

		// change
		sfg::Label::Ptr changeTip;

		sfg::Button::Ptr changeLeg;
		sfg::Label::Ptr legLabel;

		sfg::Button::Ptr changeArm;
		sfg::Label::Ptr armLabel;

		sfg::Button::Ptr changeBody;
		sfg::Label::Ptr bodyLabel;

		sfg::Button::Ptr changeShoulder;
		sfg::Label::Ptr shoulderLabel;
		sfg::Window::Ptr changeWindow;

		//save window
		sfg::Label::Ptr fileName;
		sfg::Entry::Ptr enterFileName;
		sfg::Button::Ptr save;
		sfg::Window::Ptr saveWindow;

		// load
		std::list<sfg::Box::Ptr> loadedMechs;
		sfg::Box::Ptr loadedBox;
		sfg::ScrolledWindow::Ptr loadWindow;

		// delete
		std::map<std::string,sfg::Box::Ptr> deletingMechs;
		sfg::Box::Ptr deletingBox;
		sfg::ScrolledWindow::Ptr deleteWindow;
		
		// legs
		sfg::Button::Ptr doneLegs;
		sfg::Window::Ptr legWindow;

		// body
		sfg::Button::Ptr doneBodys;
		sfg::Window::Ptr bodyWindow;

		// arm
		sfg::Button::Ptr doneArms;
		sfg::Window::Ptr armWindow;

		// shoulder
		sfg::Button::Ptr doneShoulders;
		sfg::Window::Ptr shoulderWindow;

		// body slots window
		sfg::Window::Ptr equipmentWindow;
		std::map<int,sfg::Button::Ptr> slots;
		sfg::Table::Ptr equipmentTable;
		void setEquipment();

		sfg::Widget::Ptr slotToChange;

		sfg::Window::Ptr equipmentChoiceWindow;

		// back
		enum page { main, change, load, deleteMech, leg, arm, body, shoulder, equip };
		page currentPage;

		sfg::Button::Ptr back;
		sfg::Window::Ptr backWindow;
};


