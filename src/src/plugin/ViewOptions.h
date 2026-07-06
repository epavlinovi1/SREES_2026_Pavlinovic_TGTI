#pragma once

#include <gui/View.h>
#include <gui/Label.h>
#include <gui/LineEdit.h>
#include <gui/NumericEdit.h>
#include <gui/CheckBox.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>
#include "TGTIPlugin.h"

class ViewOptions : public gui::View
{
protected:
    gui::Label _lblModelName;
    gui::LineEdit _editModelName;
    gui::Label _lblMaxIter;
    gui::NumericEdit _neMaxIter;
    gui::Label _lbldT;
    gui::NumericEdit _neDeltaTime;
    gui::Label _lblEndTime;
    gui::NumericEdit _neEndTime;
    gui::CheckBox _cbStandardGenerators;
    
    gui::Label _lblR;
    gui::Label _lblTs;
    gui::Label _lblTc;
    gui::Label _lblT3;
    gui::Label _lblT4;
    gui::Label _lblT5;
    gui::Label _lblPmax;
    gui::Label _lblPmin;
    gui::NumericEdit _neR;
    gui::NumericEdit _neTs;
    gui::NumericEdit _neTc;
    gui::NumericEdit _neT3;
    gui::NumericEdit _neT4;
    gui::NumericEdit _neT5;
    gui::NumericEdit _nePmax;
    gui::NumericEdit _nePmin;
    
    gui::GridLayout _gl;
    TGTIOptions _options;
public:
    ViewOptions()
    : _lblModelName("Model name:")
    , _lblMaxIter("Max iterations:")
    , _neMaxIter(td::int4)
    , _lbldT("dT:")
    , _neDeltaTime(td::real4, gui::LineEdit::Messages::DoNotSend, false, "dT", 4)
    , _lblEndTime("End time:")
    , _neEndTime(td::real4, gui::LineEdit::Messages::DoNotSend, false, "End time", 3)
    , _cbStandardGenerators("Standard model for other generators")
    , _lblR("R")
    , _lblTs("Ts")
    , _lblTc("Tc")
    , _lblT3("T3")
    , _lblT4("T4")
    , _lblT5("T5")
    , _lblPmax("pmax")
    , _lblPmin("pmin")
    , _neR(td::real4, gui::LineEdit::Messages::DoNotSend, false, "R", 5)
    , _neTs(td::real4, gui::LineEdit::Messages::DoNotSend, false, "Ts", 5)
    , _neTc(td::real4, gui::LineEdit::Messages::DoNotSend, false, "Tc", 5)
    , _neT3(td::real4, gui::LineEdit::Messages::DoNotSend, false, "T3", 5)
    , _neT4(td::real4, gui::LineEdit::Messages::DoNotSend, false, "T4", 5)
    , _neT5(td::real4, gui::LineEdit::Messages::DoNotSend, false, "T5", 5)
    , _nePmax(td::real4, gui::LineEdit::Messages::DoNotSend, false, "pmax", 5)
    , _nePmin(td::real4, gui::LineEdit::Messages::DoNotSend, false, "pmin", 5)
    , _gl(10, 4)
    {
        _editModelName = "Turbine Governor Type I generated model";
        _neMaxIter.setValue(td::INT4(20));
        _neDeltaTime.setValue(0.005f);
        _neEndTime.setValue(10.0f);
        _cbStandardGenerators.setChecked(true);
        _neR.setValue(0.02);
        _neTs.setValue(0.10);
        _neTc.setValue(0.45);
        _neT3.setValue(0.0);
        _neT4.setValue(0.01);
        _neT5.setValue(50.0);
        _nePmax.setValue(1.20);
        _nePmin.setValue(0.30);
        
        gui::GridComposer gc(_gl);
        gc.appendRow(_lblModelName); gc.appendCol(_editModelName, 0);
        gc.appendRow(_lblMaxIter) << _neMaxIter << _lbldT << _neDeltaTime;
        gc.appendRow(_lblEndTime) << _neEndTime;
        gc.appendRow(_cbStandardGenerators, 0);
        gc.appendRow(_lblR) << _neR << _lblTs << _neTs;
        gc.appendRow(_lblTc) << _neTc << _lblT3 << _neT3;
        gc.appendRow(_lblT4) << _neT4 << _lblT5 << _neT5;
        gc.appendRow(_lblPmax) << _nePmax << _lblPmin << _nePmin;
        setLayout(&_gl);
    }
    
    const TGTIOptions& getOptions()
    {
        _options.modelName = _editModelName.getText();
        _options.maxIter = _neMaxIter.getValue().i4Val();
        _options.dTime = _neDeltaTime.getValue().r4Val();
        _options.endTime = _neEndTime.getValue().r4Val();
        _options.useStandardForOthers = _cbStandardGenerators.isChecked();
        _options.params.R = _neR.getValue().r4Val();
        _options.params.Ts = _neTs.getValue().r4Val();
        _options.params.Tc = _neTc.getValue().r4Val();
        _options.params.T3 = _neT3.getValue().r4Val();
        _options.params.T4 = _neT4.getValue().r4Val();
        _options.params.T5 = _neT5.getValue().r4Val();
        _options.params.pmax = _nePmax.getValue().r4Val();
        _options.params.pmin = _nePmin.getValue().r4Val();
        return _options;
    }
};
