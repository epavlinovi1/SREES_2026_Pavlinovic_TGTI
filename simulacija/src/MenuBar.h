#pragma once
#include <gui/MenuBar.h> //natid bazna klasa za meni traku

class MenuBar : public gui::MenuBar
{
private:
    gui::SubMenu subFirst;//padajuci meniji
    gui::SubMenu subSimulation;

protected:
    void populateFirstMenu()
    {
        auto& items = subFirst.getItems();
        items[0].initAsQuitAppActionItem(tr("Quit"), "q"); //precica za quit
    }

    void populateSimulationMenu()//dodajemo stavke u simulation meni
    {
        auto& items = subSimulation.getItems();
        items[0].initAsActionItem(tr("start"), 10,"s"); //10 je ID 
        items[0].setAsCheckable();
        items[1].initAsActionItem(tr("reset"), 20, "r");
    }

public:
    MenuBar()
    : gui::MenuBar(2)//inicijalizacija meni trake s 2 opadajuca menija i svaki dobija ID, naziv i broj stavki
    , subFirst(10, tr("App"), 1)
    , subSimulation(20, tr("simulation"), 2)
    {
        populateFirstMenu();
        populateSimulationMenu();
        _menus[0] = &subFirst;
        _menus[1] = &subSimulation;//registrujemo menije u niz
    }
};
