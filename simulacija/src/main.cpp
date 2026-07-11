//
//  Created by Izudin Dzafic on 10 Nov 2022.
//  Copyright © 2022 IDz. All rights reserved.
//
#include <mem/LeakDetector.h> //iz natid-detekcija curenja memorije
#include <gui/WinMain.h> //zbog gui na windowsu
#include <td/StringConverter.h> //natid koristi za konverziju stringova
#include "Application.h"

int main(int argc, const char * argv[])
{
    Application app(argc, argv);
    
    auto appProperties = app.getProperties(); //ucitava podesavanja aplikacije sacuvana u OS
    td::String trLang = appProperties->getValue("translation", "EN"); //en kao default jezik
    app.init(trLang);
    return app.run();//pokrece gui dok korisnik ne zatvori prozor
}
