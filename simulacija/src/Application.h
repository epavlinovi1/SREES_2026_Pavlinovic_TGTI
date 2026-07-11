//
//  Created by Izudin Dzafic on 10 Nov 2022.
//  Copyright © 2022 IDz. All rights reserved.
//


#pragma once //ucitaj header samo jednom cak i ako je pozvan tj. ukljucen vise puta
#include <gui/Application.h>
#include "MainWindow.h"

class Application : public gui::Application //korisnici klase application vide sve javne metode bazne klase gui::application
{
protected:
    
    gui::Window* createInitialWindow() override
    {
        return new MainWindow;
    }
    
public:
    Application(int argc, const char** argv)
    : gui::Application(argc, argv)
    {//prazno jer bazna klasa radi sve sto treba
    }
};
