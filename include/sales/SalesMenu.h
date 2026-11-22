#ifndef SALESMENU_H
#define SALESMENU_H

#include "StadiumShop.h"
#include "personnel/Spectator.h"

#include <map>
#include <string>

// Opens the ticketing and stadium shop menu.
void runSalesMenu(StadiumShop& shop);

// Same menu, but the registered spectators stay available after the menu closes.
void runSalesMenu(StadiumShop& shop,
    std::map<std::string, Spectator>& spectators);

#endif