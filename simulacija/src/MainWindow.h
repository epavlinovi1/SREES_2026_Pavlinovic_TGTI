#pragma once
#include "MainView.h"
#include "MenuBar.h"
#include "ToolBar.h"
#include <functional>
#include <gui/Window.h>

class MainWindow : public gui::Window
{
protected:
    gui::Image _imgStart;
    gui::Image _imgStop;
    gui::Image _imgReset;
    MenuBar _mainMenuBar; //instance klasa
    ToolBar _toolBar;
    std::function<void()> _fnUpdateMenuAndTB;//azuriranje menija i toolbara
    MainView _mainView;

    void updateMenuAndTB()
    {
        bool isRunning = _mainView.isRunning(); //pita mainView da li simulacija trenutno teče
        gui::MenuItem* pMenuItem = _mainMenuBar.getItem(20, 0, 0, 10);//(menuID,firstSubMenuID,lastSubMenuID,actionID)
        pMenuItem->setChecked(isRunning);//kvacica kada simulacija tece

        gui::ToolBarItem* pTBItem = _toolBar.getItem(20, 0, 0, 10);
        if (pTBItem) //toolbar dugme mijenja izgled ovisno o stanju simulacije
        {
            if (isRunning)
            {
                pTBItem->setImage(&_imgStop);
                pTBItem->setLabel(tr("stop"));
                pTBItem->setTooltip(tr("stopTT"));
            }
            else
            {
                pTBItem->setImage(&_imgStart);
                pTBItem->setLabel(tr("start"));
                pTBItem->setTooltip(tr("startTT"));
            }
        }
    }

    bool shouldClose() override //error ako simulacija tece a korisnik zeli zatvoriti prozor
    {
        if (_mainView.isRunning())
        {
            showAlert(tr("closeNOK"), tr("closeErr"));
            return false;
        }
        return true;
    }

    bool onActionItem(gui::ActionItemDescriptor& aiDesc) override//kada korisnik klikne stavku menija ili toolbara
    {
        auto [menuID, firstSubMenuID, lastSubMenuID, actionID] = aiDesc.getIDs();

        if (menuID == 20 && firstSubMenuID == 0 && lastSubMenuID == 0)
        {
            switch (actionID)
            {
                case 10:
                    _mainView.startStop();
                    return true;
                case 20:
                    _mainView.reset();
                    return true;
            }
        }
        return false;
    }

public:
    MainWindow()
    : gui::Window(gui::Size(1200, 760))
    , _imgStart(":start")//definirani u res/main.xml
    , _imgStop(":stop")
    , _imgReset(":reset")
    , _toolBar(&_imgStart, &_imgReset)//stop nije tu, dodajemo ga dinamicki
    , _fnUpdateMenuAndTB(std::bind(&MainWindow::updateMenuAndTB, this))
    , _mainView(&_fnUpdateMenuAndTB)//reakcija na promjenu stanja simulacije
    {
        setTitle(tr("appTitle"));
        _mainMenuBar.setAsMain(this);
        setToolBar(_toolBar);
        setCentralView(&_mainView);
    }
};
