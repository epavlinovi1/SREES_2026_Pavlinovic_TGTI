#pragma once
#include "PlotView.h"
#include "SimulationParams.h"
#include "TurbineGovernorModel.h"
#include "ViewProperties.h"
#include <gui/SplitterLayout.h>
#include <gui/Timer.h>
#include <gui/View.h>
#include <functional>

const float FPS = 60.0f;//
const float dT = 1 / FPS;

class MainView : public gui::View
{
protected:
    gui::SplitterLayout _splitter;//dijeli view na dva dijela s pomicnom granicom, lijevo plot desno kontrole
    PlotView _plotView;
    ViewProperties _properties;
    gui::Timer _timer;
    SimulationParams _simulationParams;
    TurbineGovernorModel _model;
    std::function<void()>* _pUpdateMenuAndTB;
    std::function<void()> _fnParamsChanged;
    size_t _visiblePoints = 0;//koliko tacaka je trenutno vidljivo na grafiku

    void runSimulation(bool keepProgress = false)//pokrece numericki proracun i proslijedjuje rezultate PlotView-u za crtanje
    {
        size_t oldVisiblePoints = _visiblePoints;
        _model.setParams(_simulationParams);
        _model.solve();
        _plotView.setData(&_model.getPoints(), _simulationParams);
        if (keepProgress)//ako se parametri promijene u toku simulacije prethodni dio ostaje a dalje se proracuna
        {
            _visiblePoints = oldVisiblePoints;
            size_t total = _plotView.getNoOfPoints();
            if (_visiblePoints < 1)
                _visiblePoints = 1;
            if (_visiblePoints > total)
                _visiblePoints = total;
            _plotView.setVisiblePoints(_visiblePoints);
        }
        else
        {
            _visiblePoints = _plotView.getNoOfPoints();
        }
    }

    void onParamsChanged()
    {
        runSimulation(_timer.isRunning());
    }

public:
    MainView(std::function<void()>* pUpdateMenuAndTB)//mijenja start/stop
    : _splitter(gui::SplitterLayout::Orientation::Horizontal, gui::SplitterLayout::AuxiliaryCell::Second)//dijeli view na desni i lijevi, s tim da je desni manji i fiksne velicine
    , _timer(this, dT, false)//onTimer() sa intervalom dT i ceka start
    , _pUpdateMenuAndTB(pUpdateMenuAndTB)//sprema pointer na callback iz MainWindow
    , _fnParamsChanged(std::bind(&MainView::onParamsChanged, this))//novi callable, poziva ga ViewPropertioes
    {
        setMargins(0, 0, 0, 0);//uklanja margine oko MainView
        _splitter.setContent(_plotView, _properties);
        _properties.setCallback(&_simulationParams, &_fnParamsChanged);//(gdje se upisuju vrijednosti slidera, sta da pozove kada se vrijednos promijeni)
        setLayout(&_splitter);//splitter kao layout
        runSimulation();//kada se pokrene imamo rezultate sa default parametrima
    }

    bool isRunning() const//poziva se da MainView zna da li da pokaze  start ili stop
    {
        return _timer.isRunning();
    }

    void startStop()//resetuje animaciju, vraca na pocetak i pokrece tajmer
    {
        if (_timer.isRunning())
        {
            _timer.stop();
            (*_pUpdateMenuAndTB)();
            return;
        }

        runSimulation();
        _visiblePoints = 1;
        _plotView.setVisiblePoints(_visiblePoints);
        _timer.start();
        (*_pUpdateMenuAndTB)();
    }

    void reset()
    {
        _timer.stop();
        runSimulation();
        (*_pUpdateMenuAndTB)();
    }

    bool onTimer(gui::Timer* pTimer) override//poziva se 60 puta u sekundi dok timer tece
    {
        size_t total = _plotView.getNoOfPoints();
        if (_visiblePoints >= total)
        {
            _timer.stop();
            (*_pUpdateMenuAndTB)();
            return true;
        }

        _visiblePoints += 12;//svaki timer event pomijera animaciju za 12 tacaka tj 3.6s realne simulacije po sekundi ekranskog vremena
        if (_visiblePoints > total)
            _visiblePoints = total;
        _plotView.setVisiblePoints(_visiblePoints);
        return true;
    }
};
/*Callable  = funkcija koja može biti pozvana
Callback  = callable dat drugome na čuvanje
Poziv     = trenutak izvršavanja te funkcije*/