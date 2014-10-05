#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <windows.h>
#include <shlobj.h>
#include <dirent.h>

#include "buildMenu.h"
#include "Game.h"
#include "images.h"
#include "utils.h"

buildMenu::buildMenu()
{
	Game::myConfig.loadFromFile();

	menuResult = stillBuilding;

	//main
	newSpec = sfg::Button::Create( "New mech" );
	newSpec->SetId("new");
    newSpec->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::newOrLoad, this);

	loadSpec = sfg::Button::Create( "Load mech" );
	loadSpec->SetId("load");
    loadSpec->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::newOrLoad, this);

	deleteSpec = sfg::Button::Create( "Delete mech" );
	deleteSpec->SetId("delete");
    deleteSpec->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::newOrLoad, this);

	sfg::Box::Ptr newEditBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );
    newEditBox->Pack( newSpec, false );
	newEditBox->Pack( loadSpec, false );
	newEditBox->Pack( deleteSpec, false );

	mainWindow = sfg::Window::Create();
	mainWindow->SetTitle( "build Test" );
    mainWindow->Add(newEditBox);
	mainWindow->SetStyle(0);
	Game::desktop.Add(mainWindow);
	mainWindow->Show(false);

	//change-----
	changeTip = sfg::Label::Create( "Click on a part name to change it");

	legLabel = sfg::Label::Create( "Legs: ");
	changeLeg = sfg::Button::Create(workingSpec.legName);
    changeLeg->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::changeLegs, this);

	armLabel = sfg::Label::Create( "Arms: ");
	changeArm = sfg::Button::Create(workingSpec.armName);
    changeArm->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::changeArms, this);

	bodyLabel = sfg::Label::Create( "Body: ");
	changeBody = sfg::Button::Create(workingSpec.bodyName);
    changeBody->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::changeBodys, this);

	shoulderLabel = sfg::Label::Create( "Shoulder: ");
	changeShoulder = sfg::Button::Create(workingSpec.shoulderName);
    changeShoulder->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::changeShoulders, this);

	sfg::Table::Ptr mechTable( sfg::Table::Create() );
	mechTable->Attach( legLabel, sf::Rect<sf::Uint32>( 0, 0, 1, 1 ), sfg::Table::FILL, sfg::Table::FILL );
	mechTable->Attach( changeLeg, sf::Rect<sf::Uint32>( 1, 0, 1, 1 ) );
	mechTable->Attach( armLabel, sf::Rect<sf::Uint32>( 0, 1, 1, 1 ), sfg::Table::FILL, sfg::Table::FILL );
	mechTable->Attach( changeArm, sf::Rect<sf::Uint32>( 1, 1, 1, 1 ) );
	mechTable->Attach( bodyLabel, sf::Rect<sf::Uint32>( 0, 2, 1, 1 ), sfg::Table::FILL, sfg::Table::FILL );
	mechTable->Attach( changeBody, sf::Rect<sf::Uint32>( 1, 2, 1, 1 ) );
	mechTable->Attach( shoulderLabel, sf::Rect<sf::Uint32>( 0, 3, 1, 1 ), sfg::Table::FILL, sfg::Table::FILL );
	mechTable->Attach( changeShoulder, sf::Rect<sf::Uint32>( 1, 3, 1, 1 ) );

	sfg::Box::Ptr changeBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );
	changeBox->Pack( changeTip, false );
    changeBox->Pack( mechTable, false );

	changeWindow = sfg::Window::Create();
    changeWindow->SetTitle( "build Test" );
    changeWindow->Add( changeBox );
	changeWindow->SetStyle(0);
	Game::desktop.Add(changeWindow);
	changeWindow->Show(false);

	//save
	sfg::Box::Ptr fileNameBox = sfg::Box::Create( sfg::Box::HORIZONTAL, 5.0f );
	fileName = sfg::Label::Create( "Mech Name: " );
	enterFileName = sfg::Entry::Create();
	sf::Vector2f size (200, 25);
	enterFileName->SetRequisition(size);
	fileNameBox->Pack(fileName,true);
	fileNameBox->Pack(enterFileName,true);

	save = sfg::Button::Create( "Save" );
    save->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::clickedSave, this);

	sfg::Box::Ptr saveBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );
	saveBox->Pack( fileNameBox, false );
	saveBox->Pack( save, false );
	saveWindow = sfg::Window::Create();
    saveWindow->SetTitle( "build Test" );
    saveWindow->Add( saveBox );
	saveWindow->SetStyle(0);
	Game::desktop.Add(saveWindow);
	saveWindow->Show(false);
	sf::Vector2f savePos (	Game::myConfig.resolution.x - saveWindow->GetClientRect().width  - 20.0f ,
							Game::myConfig.resolution.y - saveWindow->GetClientRect().height   );
	saveWindow->SetPosition(savePos);


	// load
	loadedBox = sfg::Box::Create(sfg::Box::VERTICAL);

	loadWindow = sfg::ScrolledWindow::Create();
	loadWindow->SetScrollbarPolicy( sfg::ScrolledWindow::HORIZONTAL_AUTOMATIC | sfg::ScrolledWindow::VERTICAL_AUTOMATIC );
	loadWindow->AddWithViewport( loadedBox );
	loadWindow->SetRequisition( sf::Vector2f( (float)Game::myConfig.resolution.x/2, (float)Game::myConfig.resolution.y - 150.f ) );


	Game::desktop.Add(loadWindow);
	loadWindow->Show(false);

	// delete
	deletingBox = sfg::Box::Create(sfg::Box::VERTICAL);

	deleteWindow = sfg::ScrolledWindow::Create();
	deleteWindow->SetScrollbarPolicy( sfg::ScrolledWindow::HORIZONTAL_AUTOMATIC | sfg::ScrolledWindow::VERTICAL_AUTOMATIC );
	deleteWindow->AddWithViewport( deletingBox );
	deleteWindow->SetRequisition( sf::Vector2f( (float)Game::myConfig.resolution.x/2, (float)Game::myConfig.resolution.y - 150.f ) );


	Game::desktop.Add(deleteWindow);
	deleteWindow->Show(false);

	//legs-----
	sfg::Box::Ptr legBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );	
	for(std::map<std::string,sf::Texture*>::iterator it = images::allLegs.begin(); it !=  images::allLegs.end(); it++)
	{
		sfg::Button::Ptr newLeg = sfg::Button::Create(it->first);
		newLeg->SetId(it->first);
		newLeg->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::selectLeg, this);

		//sfg::Image::Ptr img = sfg::Image::Create(it->second->copyToImage());
		//newLeg->SetImage(img);
		
		legBox->Pack(newLeg, false );
	}

	doneLegs = sfg::Button::Create( "Done" );
	doneLegs->SetId("doneLeg");
    doneLegs->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::doneChanging, this);
	legBox->Pack( doneLegs, false );

	legWindow = sfg::Window::Create();
    legWindow->SetTitle( "leg Test" );
    legWindow->Add( legBox );
	legWindow->SetStyle(0);
	Game::desktop.Add(legWindow);
	legWindow->Show(false);
	
	//bodies-----
	sfg::Box::Ptr bodyBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );	
	for(std::map<std::string,sf::Texture*>::iterator it = images::allBodys.begin(); it !=  images::allBodys.end(); it++)
	{
		sfg::Button::Ptr newBody = sfg::Button::Create(it->first);
		newBody->SetId(it->first);
		newBody->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::selectBody, this);

		sfg::Image::Ptr img = sfg::Image::Create(it->second->copyToImage());
		//newBody->SetImage(img);

		bodyBox->Pack(newBody, false );
	}

	doneBodys = sfg::Button::Create( "Done" );
	doneBodys->SetId("doneBody");
    doneBodys->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::doneChanging, this);
	bodyBox->Pack( doneBodys, false );

	bodyWindow = sfg::Window::Create();
    bodyWindow->SetTitle( "body Test" );
    bodyWindow->Add( bodyBox );
	bodyWindow->SetStyle(0);
	Game::desktop.Add(bodyWindow);
	bodyWindow->Show(false);


	//arms
	sfg::Box::Ptr armBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );	
	for(std::map<std::string,sf::Texture*>::iterator it = images::allArms.begin(); it !=  images::allArms.end(); it++)
	{
		sfg::Button::Ptr newArm = sfg::Button::Create(it->first);
		newArm->SetId(it->first);
		newArm->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::selectArm, this);

		sfg::Image::Ptr img = sfg::Image::Create(it->second->copyToImage());
		//newArm->SetImage(img);

		armBox->Pack(newArm, false );
	}

	doneArms = sfg::Button::Create( "Done" );
	doneArms->SetId("doneArm");
    doneArms->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::doneChanging, this);
	armBox->Pack( doneArms, false );

	armWindow = sfg::Window::Create();
    armWindow->SetTitle( "arm Test" );
    armWindow->Add( armBox );
	armWindow->SetStyle(0);
	Game::desktop.Add(armWindow);
	armWindow->Show(false);

	//Shoulders-----
	sfg::Box::Ptr shoulderBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );	
	for(std::map<std::string,sf::Texture*>::iterator it = images::allShoulders.begin(); it !=  images::allShoulders.end(); it++)
	{
		sfg::Button::Ptr newShoulder = sfg::Button::Create(it->first);
		newShoulder->SetId(it->first);
		newShoulder->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::selectShoulder, this);

		sfg::Image::Ptr img = sfg::Image::Create(it->second->copyToImage());
		//newShoulder->SetImage(img);

		shoulderBox->Pack(newShoulder, false );
	}


	doneShoulders = sfg::Button::Create( "Done" );
	doneShoulders->SetId("doneShoulder");
    doneShoulders->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::doneChanging, this);
	shoulderBox->Pack( doneShoulders, false );

	shoulderWindow = sfg::Window::Create();
    shoulderWindow->SetTitle( "shoulder Test" );
    shoulderWindow->Add( shoulderBox );
	shoulderWindow->SetStyle(0);
	Game::desktop.Add(shoulderWindow);
	shoulderWindow->Show(false);

	// back
	back = sfg::Button::Create("Back");
	back->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::prevPage, this);
	backWindow = sfg::Window::Create();
	backWindow->Add(back);
	backWindow->SetStyle(0);
	Game::desktop.Add(backWindow);
	backWindow->Show(false);
	sf::Vector2f windowEdge ( 0 , Game::myConfig.resolution.y - backWindow->GetClientRect().height  - 20.0f );
	backWindow->SetPosition(windowEdge);

	// slots
	equipmentTable = sfg::Table::Create();

	equipmentWindow = sfg::Window::Create();
    equipmentWindow->Add(equipmentTable);
	equipmentWindow->SetTitle("Body Schematic");

	Game::desktop.Add(equipmentWindow);
	equipmentWindow->Show(false);
	equipmentWindow->SetStyle(0);
	sf::Vector2f pos( 0 , changeWindow->GetClientRect().height + 10.0f );
	equipmentWindow->SetPosition(pos);
	sf::Vector2f eqSize( changeWindow->GetClientRect().width , 90 );
	equipmentWindow->SetRequisition(eqSize);

	//equipment choices-----
	sfg::Box::Ptr equipBox = sfg::Box::Create( sfg::Box::VERTICAL, 5.0f );	
	std::list<std::string> choices;

	// spaces in name might not work
	choices.push_front("Radar");
	choices.push_front("Sheilding");
	choices.push_front("Fuel Tank");
	choices.push_front("Improved Hull");
	choices.push_front("Jump Jets");

	for(std::list<std::string>::iterator it = choices.begin(); it != choices.end(); it++)
	{
		sfg::Button::Ptr newChoice = sfg::Button::Create(*it);
		newChoice->SetId(*it);
		newChoice->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::selectEquip, this);
		equipBox->Pack(newChoice, false );
	}

	sfg::Button::Ptr doneEquip = sfg::Button::Create( "Done" );
	doneEquip->SetId("doneEquip");
    doneEquip->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::doneChanging, this);
	equipBox->Pack( doneEquip, false );

	equipmentChoiceWindow = sfg::Window::Create();
    equipmentChoiceWindow->Add(equipBox);
	equipmentChoiceWindow->SetTitle("Chose Equipment");
	equipmentChoiceWindow->SetStyle(0);
	Game::desktop.Add(equipmentChoiceWindow);
	equipmentChoiceWindow->Show(false);

}



void buildMenu::prevPage()
{
	switch(currentPage)
	{
	case main:
		mainWindow->Show(false);
		backWindow->Show(false);
		menuResult = done;
		break;
	case change:
		changeWindow->Show(false);
		mainWindow->Show(true);
		currentPage = main;
		break;
	case load:
		loadWindow->Show(false);
		mainWindow->Show(true);
		currentPage = main;
		break;
	case deleteMech:
		deleteWindow->Show(false);
		mainWindow->Show(true);
		currentPage = main;
		break;
	case leg:
		legWindow->Show(false);
		changeWindow->Show(true);
		currentPage = change;
		break;
	case arm:
		armWindow->Show(false);
		changeWindow->Show(true);
		currentPage = change;
		break;
	case shoulder:
		shoulderWindow->Show(false);
		changeWindow->Show(true);
		currentPage = change;
		break;
	case body:
		bodyWindow->Show(false);
		changeWindow->Show(true);
		currentPage = change;
		break;
	case equip:
		equipmentChoiceWindow->Show(false);
		changeWindow->Show(true);
		currentPage = change;
		break;
	}
}


void buildMenu::changeLegs()
{
	changeWindow->Show(false);
	legWindow->Show(true);
	currentPage = leg;
}

void buildMenu::changeBodys()
{
	changeWindow->Show(false);
	bodyWindow->Show(true);
	currentPage = body;
}

void buildMenu::changeArms()
{
	changeWindow->Show(false);
	armWindow->Show(true);
	currentPage = arm;
}

void buildMenu::changeShoulders()
{
	changeWindow->Show(false);
	shoulderWindow->Show(true);
	currentPage = shoulder;
}

void buildMenu::start() 
{
	mainWindow->Show(true);
	backWindow->Show(true);
	menuResult = stillBuilding;
	currentPage = main;
}

void buildMenu::selectLeg() 
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );
	tempSpec.legName = widget->GetId();

	std::cout << "leg: " << tempSpec.legName << std::endl;
}

void buildMenu::selectBody() 
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );
	tempSpec.bodyName = widget->GetId();

	std::cout << "body: " << tempSpec.bodyName << std::endl;
}

void buildMenu::selectArm() 
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );
	tempSpec.armName = widget->GetId();

	std::cout << "arm: " << tempSpec.armName << std::endl;
}

void buildMenu::selectShoulder() 
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );
	tempSpec.shoulderName = widget->GetId();

	std::cout << "shoulder: " << tempSpec.shoulderName << std::endl;
}

void buildMenu::doneChanging()
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );

	if(widget->GetId() == "doneLeg")
	{
		legWindow->Show(false);
		workingSpec.legName = tempSpec.legName;
		changeLeg->SetLabel(workingSpec.legName);
	}
	else if (widget->GetId() == "doneBody")
	{
		bodyWindow->Show(false);
		workingSpec.bodyName = tempSpec.bodyName ;
		changeBody->SetLabel(workingSpec.bodyName);

		setEquipment();
	}
	else if (widget->GetId() == "doneArm")
	{
		armWindow->Show(false);
		workingSpec.armName = tempSpec.armName ;
		changeArm->SetLabel(workingSpec.armName);
	}
	else if (widget->GetId() == "doneShoulder")
	{
		shoulderWindow->Show(false);
		workingSpec.shoulderName = tempSpec.shoulderName ;
		changeShoulder->SetLabel(workingSpec.shoulderName);
	}
	else if (widget->GetId() == "doneEquip")
	{
		equipmentChoiceWindow->Show(false);
	}

	changeWindow->Show(true);
	currentPage = change;

}

void buildMenu::newOrLoad()
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );

	std::list<sfg::Box::Ptr>::iterator itr = loadedMechs.begin();
	while(itr != loadedMechs.end())
	{
		loadedBox->Remove(*itr); 
		itr++;
	}
	loadedMechs.clear();


	std::map<std::string,sfg::Box::Ptr>::iterator it = deletingMechs.begin();
	while(it != deletingMechs.end())
	{
		deletingBox->Remove(it->second); 
		it++;
	}
	deletingMechs.clear(); 


	if(widget->GetId() == "new")
	{
		changeWindow->Show(true);
		mainWindow->Show(false);
		setEquipment();
		currentPage = change;
	}
	else if (widget->GetId() == "load")
	{
		if(loadSpecs())
		{
			loadWindow->Show(true);
			mainWindow->Show(false);
			currentPage = load;
		}
		else
		{
			Game::messageWindow.displayMessage("Error loading mech files");
		}
	}
	else if (widget->GetId() == "delete")
	{
		if(loadSpecsDelete())
		{
			deleteWindow->Show(true);
			mainWindow->Show(false);
			currentPage = deleteMech;
		}
		else
		{
			Game::messageWindow.displayMessage("Error loading mech files");
		}
	}
}

void buildMenu::clickedSave()
{
	std::string mechName;
	std::stringstream field( static_cast<std::string>( enterFileName->GetText() ) );
	field >> mechName;

	if ( workingSpec.saveToFile(mechName) )
	{
		std::string messageText = "Successfully saved ";
		messageText.append(mechName);
		Game::messageWindow.displayMessage(messageText);
	}
	else
	{
		std::string messageText = "Error saving ";
		messageText.append(mechName);
		Game::messageWindow.displayMessage(messageText);
	}
}

void buildMenu::clickedMech()
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );

	std::string mechId = widget->GetId();
	if ( workingSpec.loadFromFile(mechId) )
	{
		changeLeg->SetLabel(workingSpec.legName);
		changeBody->SetLabel(workingSpec.bodyName);
		changeArm->SetLabel(workingSpec.armName);
		changeShoulder->SetLabel(workingSpec.shoulderName);

		setEquipment();
		for(std::map<int,std::string>::iterator it = workingSpec.equipment.begin(); it != workingSpec.equipment.end(); it++)
		{
			slots.find(it->first)->second->SetLabel(it->second);
		}


		mechId.erase(mechId.length()-4,mechId.length());
		enterFileName->SetText(mechId);

		loadWindow->Show(false);
		changeWindow->Show(true);
		
		currentPage = change;
	}
	else
	{
		mechId.erase(mechId.length()-4,mechId.length());

		std::string messageText = "Error loading ";
		messageText.append(mechId);
		Game::messageWindow.displayMessage(messageText);
	}

}

void buildMenu::clickedMechDelete()
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );

	std::string mechId = widget->GetId();
	if ( workingSpec.deleteFile(mechId) )
	{
		deletingMechs.find(mechId)->second->Show(false);
	}
	else
	{
		mechId.erase(mechId.length()-4,mechId.length());

		std::string messageText = "Error deleting ";
		messageText.append(mechId);
		Game::messageWindow.displayMessage(messageText);
	}

}

buildMenu::result buildMenu::Show(sf::RenderWindow& renderWindow) 
{
	// We're not using SFML to render anything in this program, so reset OpenGL
	// states. Otherwise we wouldn't see anything.
	renderWindow.resetGLStates();

	// Main loop!
	sf::Event event1;
	sf::Clock clock;


	while( renderWindow.isOpen() ) 
	{

		
		if(this->currentPage == change)
		{
			equipmentWindow->Show(true);
			saveWindow->Show(true);
			Game::desktop.BringToFront(equipmentWindow);
		}
		else
		{
			equipmentWindow->Show(false);
			saveWindow->Show(false);
		}

		// Event processing.
		while( renderWindow.pollEvent( event1 ) )
		{
			Game::desktop.HandleEvent( event1 );

			// If window is about to be closed, leave program.
			if( event1.type == sf::Event::Closed ) 
			{
				return quit;
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

		if(menuResult != stillBuilding)
			return menuResult;

	}

	return menuResult;

}

void buildMenu::setEquipment()
{
	bodyStat stat;
	stat.loadFromFile(workingSpec.bodyName);

	for(std::map<int,sfg::Button::Ptr>::iterator it = slots.begin(); it !=  slots.end(); it++)
	{
		it->second->Show(false);
		equipmentTable->Remove(it->second);
	}
	slots.clear();

	for(int i=0; i < stat.slots; i++)
	{
		int row = i/3;
		int col = i%3;

		sfg::Button::Ptr slot = sfg::Button::Create();
		slot->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::changeEquip, this);

		slots.insert( std::pair<int,sfg::Button::Ptr>(i,slot) );
		slot->SetId(utils::toString(i));

		equipmentTable->Attach( slot, sf::Rect<sf::Uint32>( col, row, 1, 1 ) );

	}
	equipmentWindow->Refresh();
	equipmentTable->Refresh();
}

bool buildMenu::loadSpecs()
{
	char path[ MAX_PATH ];
	if (SHGetFolderPathA( NULL, CSIDL_PROFILE, NULL, 0, path ) != S_OK)
		return false;

	std::string saveDirectory = path;
	saveDirectory.append("\\Documents\\My Games\\RADgame\\saves\\mechs\\");

	std::cout << "Saving to: " << saveDirectory << std::endl;

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
				loadedMech->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::clickedMech, this);
				loadedMech->SetId(fileName);

				// chop off .txt from button
				fileName.erase(fileName.length()-4,fileName.length());
				loadedMech->SetLabel(fileName);

				sfg::Box::Ptr tempBox = sfg::Box::Create();
				tempBox->Pack(loadedMech,false,false);

				loadedBox->Pack(tempBox,false,false);
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

bool buildMenu::loadSpecsDelete()
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
				loadedMech->GetSignal( sfg::Widget::OnLeftClick ).Connect( &buildMenu::clickedMechDelete, this);
				loadedMech->SetId(fileName);

				

				sfg::Box::Ptr tempBox = sfg::Box::Create();
				tempBox->Pack(loadedMech,false,false);

				deletingBox->Pack(tempBox,false,false);
				deletingMechs.insert( std::pair<std::string,sfg::Box::Ptr>(fileName,tempBox) ); 

				// chop off .txt from button
				fileName.erase(fileName.length()-4,fileName.length());
				loadedMech->SetLabel(fileName);
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

void buildMenu::changeEquip()
{
	changeWindow->Show(false);
	equipmentChoiceWindow->Show(true);
	currentPage = equip;

	slotToChange = sfg::Context::Get().GetActiveWidget();

	workingSpec.equipment.erase(utils::toInt(slotToChange->GetId()));
}

void buildMenu::selectEquip()
{
	sfg::Widget::Ptr widget( sfg::Context::Get().GetActiveWidget() );

	int num = utils::toInt(slotToChange->GetId());

	slots.find(num)->second->SetLabel(widget->GetId());
	workingSpec.equipment.insert( std::pair<int,std::string>(num,widget->GetId()));
}