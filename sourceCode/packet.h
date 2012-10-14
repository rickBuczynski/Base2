#pragma once
#include "visibleGameObject.h"
#include "networkEvent.h"
#include "lobbyEvent.h"
#include "mechSpec.h"
#include <SFML/Network.hpp>

sf::Packet& operator <<(sf::Packet& Packet, const VisibleGameObject& _vgo);


sf::Packet& operator >>(sf::Packet& Packet, VisibleGameObject& _vgo);


sf::Packet& operator <<(sf::Packet& Packet, const networkEvent& action);

sf::Packet& operator <<(sf::Packet& Packet, const lobbyEvent& action);

sf::Packet& operator <<(sf::Packet& Packet, const mechSpec& mechSpec);

sf::Packet& operator >>(sf::Packet& Packet, mechSpec& mechSpec);