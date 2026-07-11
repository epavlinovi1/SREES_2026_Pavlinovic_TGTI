//panel sa kontrolama

#pragma once
#include "SimulationParams.h"
#include <gui/GridComposer.h>
#include <gui/GridLayout.h>
#include <gui/Label.h>
#include <gui/NumericEdit.h>
#include <gui/Slider.h>
#include <gui/VerticalLayout.h>
#include <gui/View.h>
#include <functional>

class ViewProperties : public gui::View
{
protected:
    gui::VerticalLayout _vl;
    gui::GridLayout _gl;

    gui::Label _lblTitle;
    gui::Label _lblR;
    gui::Label _lblTs;
    gui::Label _lblTc;
    gui::Label _lblT3;
    gui::Label _lblT4;
    gui::Label _lblT5;
    gui::Label _lblGridFrequency;

    gui::Slider _slR;
    gui::Slider _slTs;
    gui::Slider _slTc;
    gui::Slider _slT3;
    gui::Slider _slT4;
    gui::Slider _slT5;
    gui::Slider _slGridFrequency;

    gui::Label _lblValR;
    gui::NumericEdit _edR;
    gui::Label _lblValTs;
    gui::NumericEdit _edTs;
    gui::Label _lblValTc;
    gui::NumericEdit _edTc;
    gui::Label _lblValT3;
    gui::NumericEdit _edT3;
    gui::Label _lblValT4;
    gui::NumericEdit _edT4;
    gui::Label _lblValT5;
    gui::NumericEdit _edT5;
    gui::Label _lblValGridFrequency;
    gui::NumericEdit _edGridFrequency;

    //inicijalizacija pointera jer mijenjamo vrijednosi u MainView
    SimulationParams* _pParams = nullptr;
    std::function<void()>* _pOnChanged = nullptr;

    void refreshEdits()//update read-only brojeva kod slidera
    {
        if (!_pParams)
            return;
        td::Decimal4 dec4 = _pParams->r; //broj sa 4 decimale iz NumericEdit(natID)
        _edR.setValue(dec4);
        td::Decimal2 dec2 = _pParams->ts;
        _edTs.setValue(dec2);
        dec2 = _pParams->tc;
        _edTc.setValue(dec2);
        dec2 = _pParams->t3;
        _edT3.setValue(dec2);
        dec2 = _pParams->t4;
        _edT4.setValue(dec2);
        dec2 = _pParams->t5;
        _edT5.setValue(dec2);
        dec2 = _pParams->gridFrequency;
        _edGridFrequency.setValue(dec2);
    }

    void populateSimulationData()//update slidera
    {
        if (!_pParams)
            return;
        _pParams->r = _slR.getValue();
        _pParams->ts = _slTs.getValue();
        _pParams->tc = _slTc.getValue();
        _pParams->t3 = _slT3.getValue();
        _pParams->t4 = _slT4.getValue();
        _pParams->t5 = _slT5.getValue();
        _pParams->gridFrequency = _slGridFrequency.getValue();
        _pParams->omegaStep = _pParams->gridFrequency / _pParams->nominalFrequency - _pParams->omegaRef;//pretvatamo u pu
        refreshEdits();
    }

    bool onChangedValue(gui::Slider* pSlider) override //poziva se svaki put kada korisnik promijeni parametre i callback poziva runSimulation() u MainView
    {
        populateSimulationData();
        if (_pOnChanged)
            (*_pOnChanged)();//dereferencirani poziv funkcije
        return true;
    }

public:
    ViewProperties()//konstruktor, inicijalizacija labeča i slidera
    : _vl(24)
    , _gl(7, 2)
    , _lblTitle(tr("lblParams"))
    , _lblR(tr("lblR"))
    , _lblTs(tr("lblTs"))
    , _lblTc(tr("lblTc"))
    , _lblT3(tr("lblT3"))
    , _lblT4(tr("lblT4"))
    , _lblT5(tr("lblT5"))
    , _lblGridFrequency(tr("lblGridFrequency"))
    , _lblValR(tr("lblValR"))
    , _edR(td::decimal4)
    , _lblValTs(tr("lblValTs"))
    , _edTs(td::decimal2)
    , _lblValTc(tr("lblValTc"))
    , _edTc(td::decimal2)
    , _lblValT3(tr("lblValT3"))
    , _edT3(td::decimal2)
    , _lblValT4(tr("lblValT4"))
    , _edT4(td::decimal2)
    , _lblValT5(tr("lblValT5"))
    , _edT5(td::decimal2)
    , _lblValGridFrequency(tr("lblValGridFrequency"))
    , _edGridFrequency(td::decimal2)
    {
        setMargins(10, 10, 10, 10);
        _lblTitle.setBold();

        _slR.setRange(0.01, 0.10);//postavljanje opsega slidera
        _slR.setValue(0.02);
        _slTs.setRange(0.02, 1.00);
        _slTs.setValue(0.10);
        _slTc.setRange(0.02, 2.00);
        _slTc.setValue(0.45);
        _slT3.setRange(0.00, 2.00);
        _slT3.setValue(0.00);
        _slT4.setRange(0.01, 2.00);
        _slT4.setValue(0.01);
        _slT5.setRange(1.00, 80.00);
        _slT5.setValue(50.00);
        _slGridFrequency.setRange(47.50, 51.00);
        _slGridFrequency.setValue(49.50);

        _edR.setAsReadOnly();//vrijednosti mozemo mijenjati samo preko slidera
        _edTs.setAsReadOnly();
        _edTc.setAsReadOnly();
        _edT3.setAsReadOnly();
        _edT4.setAsReadOnly();
        _edT5.setAsReadOnly();
        _edGridFrequency.setAsReadOnly();

        _vl << _lblTitle//dodajemo kontrole u vertikalni layout jednu ispod druge
            << _lblR << _slR
            << _lblTs << _slTs
            << _lblTc << _slTc
            << _lblT3 << _slT3
            << _lblT4 << _slT4
            << _lblT5 << _slT5
            << _lblGridFrequency << _slGridFrequency;

        gui::GridComposer gc(_gl);//slaze labele i vrdijednosti
        gc.appendRow(_lblValR); gc.appendCol(_edR);
        gc.appendRow(_lblValTs); gc.appendCol(_edTs);
        gc.appendRow(_lblValTc); gc.appendCol(_edTc);
        gc.appendRow(_lblValT3); gc.appendCol(_edT3);
        gc.appendRow(_lblValT4); gc.appendCol(_edT4);
        gc.appendRow(_lblValT5); gc.appendCol(_edT5);
        gc.appendRow(_lblValGridFrequency); gc.appendCol(_edGridFrequency);
        _vl.append(_gl);
        _vl.appendSpacer();
        setLayout(&_vl);
    }
    //povezuje ViewProperties odnosno pointere sa pravim vrijednostima, poziva se iz MainView
    void setCallback(SimulationParams* pParams, std::function<void()>* pOnChanged)
    {
        _pParams = pParams;
        _pOnChanged = pOnChanged;
        populateSimulationData();
    }

    void enableControls()
    {
        _slR.disable(false);
        _slTs.disable(false);
        _slTc.disable(false);
        _slT3.disable(false);
        _slT4.disable(false);
        _slT5.disable(false);
        _slGridFrequency.disable(false);
    }

    void disableControls()
    {
        _slR.disable(true);
        _slTs.disable(true);
        _slTc.disable(true);
        _slT3.disable(true);
        _slT4.disable(true);
        _slT5.disable(true);
        _slGridFrequency.disable(true);
    }
};
/*
Korisnik pomjeri slider
        ↓
onChangedValue() — natID callback
        ↓
populateSimulationData() — čita slidere → upisuje u SimulationParams
        ↓
refreshEdits() — ažurira read-only polja
        ↓
(*_pOnChanged)() — poziva MainView::onParamsChanged()
        ↓
Nova simulacija se izračuna i nacrta*/