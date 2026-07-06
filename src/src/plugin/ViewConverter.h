#pragma once

#include <gui/View.h>
#include <gui/Label.h>
#include <gui/Button.h>
#include <gui/LineEdit.h>
#include <gui/NumericEdit.h>
#include <gui/TextEdit.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>
#include <gui/HorizontalLayout.h>
#include <gui/FileDialog.h>
#include <fo/FileOperations.h>
#include "TGTIPlugin.h"
#include "ViewOptions.h"

class ViewConverter : public gui::View
{
protected:
    sc::IPlugin* _pIPlugin = nullptr;
    sc::IPlugin::CallBack _onComplete;
    ViewOptions* _pOptions = nullptr;
    
    gui::Label _lblInput;
    gui::LineEdit _editInput;
    gui::Button _btnInput;
    gui::Label _lblOutput;
    gui::LineEdit _editOutput;
    gui::Button _btnOutput;
    gui::Label _lblConfig;
    gui::LineEdit _editConfig;
    gui::Button _btnConfig;
    gui::Label _lblGenerator;
    gui::NumericEdit _neGenerator;
    gui::Label _lblStatus;
    gui::LineEdit _editStatus;
    gui::Label _lblProgress;
    gui::LineEdit _editProgress;
    gui::TextEdit _teLog;
    gui::Button _btnInspect;
    gui::Button _btnConvert;
    gui::HorizontalLayout _hlButtons;
    gui::GridLayout _gl;
    
    ConversionProgress _progress;
    
    TGTIOptions collectOptions()
    {
        TGTIOptions options;
        if (_pOptions)
            options = _pOptions->getOptions();
        options.configFileName = _editConfig.getText();
        options.generatorNo = _neGenerator.getValue().i4Val();
        return options;
    }
    
    void handleUserActions()
    {
        _btnInput.onClick([this] {
            gui::OpenFileDialog::show(this, tr("openMatpower"), "*.m", 1101, [this](gui::FileDialog* pDlg) {
                if (pDlg->getStatus() == gui::FileDialog::Status::OK)
                    _editInput = pDlg->getFileName();
            });
        });
        
        _btnOutput.onClick([this] {
            gui::SaveFileDialog::show(this, tr("saveDmodl"), "*.dmodl", 1102, [this](gui::FileDialog* pDlg) {
                if (pDlg->getStatus() == gui::FileDialog::Status::OK)
                    _editOutput = pDlg->getFileName();
            });
        });
        
        _btnConfig.onClick([this] {
            gui::OpenFileDialog::show(this, tr("openXml"), "*.xml", 1103, [this](gui::FileDialog* pDlg) {
                if (pDlg->getStatus() == gui::FileDialog::Status::OK)
                    _editConfig = pDlg->getFileName();
            });
        });
        
        _btnInspect.onClick([this] {
            td::String inputFileName = _editInput.getText();
            if (inputFileName.isEmpty() || !fo::fileExists(inputFileName))
            {
                _editStatus = "ERROR! MATPOWER file does not exist.";
                return;
            }
            td::String content;
            if (fo::loadFileContent(inputFileName, content))
            {
                _teLog.setText(content);
                _editStatus = "MATPOWER input loaded.";
            }
        });
        
        _btnConvert.onClick([this] {
            td::String inputFileName = _editInput.getText();
            td::String outFileName = _editOutput.getText();
            if (inputFileName.isEmpty() || !fo::fileExists(inputFileName))
            {
                _editStatus = "ERROR! Empty or missing MATPOWER input.";
                return;
            }
            if (outFileName.isEmpty())
            {
                _editStatus = "ERROR! Empty output file name.";
                return;
            }
            if (_progress.running)
            {
                _editStatus = "Conversion is already running.";
                return;
            }
            
            _progress.reset();
            TGTIOptions options = collectOptions();
            td::String status;
            _progress.running = true;
            bool ok = createTGTIModel(inputFileName, outFileName, _pIPlugin, options, _progress, status);
            _progress.running = false;
            _editStatus = status;
            _editProgress = ok ? "100%" : "Failed";
            if (ok)
                _onComplete(_pIPlugin);
        });
    }
    
public:
    ViewConverter(sc::IPlugin* pIPlugin,
                  const sc::IPlugin::CallBack& onComplete)
    : _pIPlugin(pIPlugin)
    , _onComplete(onComplete)
    , _lblInput("MATPOWER case:")
    , _btnInput("...")
    , _lblOutput("Output .dmodl:")
    , _btnOutput("...")
    , _lblConfig("Configuration XML:")
    , _btnConfig("...")
    , _lblGenerator("Generator number:")
    , _neGenerator(td::int4)
    , _lblStatus("Status:")
    , _lblProgress("Progress:")
    , _btnInspect("Inspect")
    , _btnConvert("Convert")
    , _hlButtons(3)
    , _gl(8, 3)
    {
        _neGenerator.setValue(td::INT4(1));
        _editStatus.setAsReadOnly();
        _editProgress.setAsReadOnly();
        _teLog.setAsReadOnly();
        _editStatus = "Idle";
        _editProgress = "0%";
        
        gui::GridComposer gc(_gl);
        gc.appendRow(_lblInput) << _editInput << _btnInput;
        gc.appendRow(_lblOutput) << _editOutput << _btnOutput;
        gc.appendRow(_lblConfig) << _editConfig << _btnConfig;
        gc.appendRow(_lblGenerator) << _neGenerator;
        gc.appendRow(_lblStatus) << _editStatus;
        gc.appendRow(_lblProgress) << _editProgress;
        gc.appendRow(_teLog, 0);
        _hlButtons.appendSpacer() << _btnInspect << _btnConvert;
        gc.appendRow(_hlButtons, 0);
        setLayout(&_gl);
        handleUserActions();
    }
    
    void setOptions(ViewOptions* pOptions)
    {
        _pOptions = pOptions;
    }

    td::String getOutFileName() const
    {
        return _editOutput.getText();
    }
};
