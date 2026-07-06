#include "TGTIPlugin.h"

//natID klase
#include <gui/Window.h>
#include <gui/View.h>
#include <gui/Label.h>
#include <gui/LineEdit.h>
#include <gui/NumericEdit.h>
#include <gui/Button.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>
#include <gui/FileDialog.h>
#include <gui/ProgressIndicator.h>


#include <fo/FileOperations.h>//koristi se za fileExists,loadFileContent...


#include <atomic>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <thread>

static void tgtiTrace(const char* message)//gui nema debuger pa rjesavamo to 
{
    std::ofstream f("C:\\Users\\User\\Documents\\SREES TGTI 2\\TGTI\\tgti_plugin_trace.log", std::ios::app);
    if (f)//provjerava da li je file uspjesno otvoren
        f << message << "\n";
}

class SimplePluginView : public gui::View//definicija klase koja nasljeđuje gui::view
{//objekti ciji je zivtoni vijek vezan za view
protected:
    sc::IPlugin* _pIPlugin = nullptr;
    sc::IPlugin::CallBack _onComplete;//callback koji dtwin koristi kada je konverzija gotova
    
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
    gui::ProgressIndicator _piProgress;
    gui::LineEdit _editProgress;
    gui::Button _btnInspect;
    gui::Button _btnConvert;
    gui::GridLayout _gl;
    //praćenje napredka
    ConversionProgress _progress;
    std::thread _workerThread;
    std::thread _progressThread;
    std::mutex _statusMutex;
    td::String _finalStatus;
    std::atomic<bool> _conversionDone = false;
    
    void joinFinishedThreads()
    {
        if (_workerThread.joinable())
            _workerThread.join();
        if (_progressThread.joinable())
            _progressThread.join();
    }
    
    TGTIOptions collectOptions() const//prepisuje putanju do config i vrstu generatora
    {
        TGTIOptions options;
        options.configFileName = _editConfig.getText();
        options.generatorNo = _neGenerator.getValue().i4Val();
        return options;
    }
    
    void handleUserActions()
    {   //lambda funkcija koja hendla klik
        _btnInput.onClick([this] {
            gui::OpenFileDialog::show(this, "Select MATPOWER case", "*.m", 2101, [this](gui::FileDialog* pDlg) {
                if (pDlg->getStatus() == gui::FileDialog::Status::OK)//povjeri je li koristnik kliknuo OK, a ne cancel
                {
                    _editInput = pDlg->getFileName();
                    _editStatus = "MATPOWER case selected.";
                }
            });
        });
        
        _btnOutput.onClick([this] {
            gui::SaveFileDialog::show(this, "Select output .dmodl", "*.dmodl", 2102, [this](gui::FileDialog* pDlg) {
                if (pDlg->getStatus() == gui::FileDialog::Status::OK)
                {
                    _editOutput = pDlg->getFileName();
                    _editStatus = "Output file selected.";
                }
            });
        });
        
        _btnConfig.onClick([this] {
            gui::OpenFileDialog::show(this, "Select TGTI XML configuration", "*.xml", 2103, [this](gui::FileDialog* pDlg) {
                if (pDlg->getStatus() == gui::FileDialog::Status::OK)
                {
                    _editConfig = pDlg->getFileName();
                    _editStatus = "Configuration XML selected.";
                }
            });
        });
        
        _btnInspect.onClick([this] {
            td::String inputFileName = _editInput.getText();
            if (inputFileName.isEmpty())
            {
                _editStatus = "ERROR! Select a MATPOWER case first.";
                return;
            }
            if (!fo::fileExists(inputFileName))
            {
                _editStatus = "ERROR! MATPOWER case file does not exist.";
                return;
            }
            
            td::String content;
            if (!fo::loadFileContent(inputFileName, content))
            {
                _editStatus = "ERROR! Cannot read MATPOWER case file.";
                return;
            }
            if (content.find("mpc.gen") < 0)
            {
                _editStatus = "WARNING! File loaded, but mpc.gen was not found.";
                return;
            }
            _editStatus = "MATPOWER case looks readable.";
        });
        
        _btnConvert.onClick([this] {
            tgtiTrace("convert clicked");//upisuje u log
            td::String inputFileName = _editInput.getText();
            td::String outFileName = _editOutput.getText();
            if (inputFileName.isEmpty() || !fo::fileExists(inputFileName))
            {
                _editStatus = "ERROR! Select an existing MATPOWER case first.";
                return;
            }
            if (outFileName.isEmpty())
            {
                _editStatus = "ERROR! Select output .dmodl path first.";
                return;
            }
            
            td::String configFileName = _editConfig.getText();//config je opcionalan
            if (!configFileName.isEmpty() && !fo::fileExists(configFileName))
            {
                _editStatus = "ERROR! Configuration XML file does not exist.";
                return;
            }
            if (_progress.running)
            {
                _editStatus = "Conversion is already running.";
                return;
            }
            //priprema za novu konverziju
            joinFinishedThreads();
            _progress.reset();
            _progress.running = true;
            _conversionDone = false;
            _piProgress.setValue(0.0);
            _editProgress = "0%";
            _editStatus = "Conversion started.";
            //hvata nove postavke
            TGTIOptions options = collectOptions();
            _workerThread = std::thread([this, inputFileName, outFileName, options] {
                td::String status;
                tgtiTrace("before createTGTIModel file-only");
                bool ok = createTGTIModel(inputFileName, outFileName, nullptr, options, _progress, status);
                tgtiTrace(ok ? "after createTGTIModel ok" : "after createTGTIModel failed");
                {
                    std::lock_guard<std::mutex> lock(_statusMutex);
                    _finalStatus = status;
                }
                _progress.failed = !ok;
                _progress.completed = ok;
                _progress.running = false;//radi dok je ovo true
                _conversionDone = true;
            });
            
            _progressThread = std::thread([this] {
                while (_progress.running)
                {
                    int percent = _progress.percent.load();
                    if (percent < 0)
                        percent = 0;
                    if (percent > 100)
                        percent = 100;
                    char text[32];
                    std::snprintf(text, sizeof(text), "%d%%", percent);
                    _piProgress.setValue(double(percent) / 100.0);
                    _editProgress = text;
                    _editStatus = _progress.getMessage();
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));//usporava petlju zbog cpu
                }
                
                int percent = _progress.percent.load();
                if (_progress.completed)
                    percent = 100;
                char text[32];
                std::snprintf(text, sizeof(text), "%d%%", percent);
                _piProgress.setValue(double(percent) / 100.0);
                _editProgress = text;
                {
                    std::lock_guard<std::mutex> lock(_statusMutex);
                    _editStatus = _finalStatus;
                }
            });
        });
    }
    
public:
    SimplePluginView(sc::IPlugin* pIPlugin,
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
    , _piProgress(gui::DataCtrl::Orientation::Horizontal, true)
    , _btnInspect("Inspect")
    , _btnConvert("Convert")
    , _gl(8, 3)
    {
        _neGenerator.setValue(td::INT4(1));
        _editStatus.setAsReadOnly();
        _editProgress.setAsReadOnly();
        _editStatus = "GUI loaded.";
        _editProgress = "0%";
        _piProgress.setValue(0.0);
        
        gui::GridComposer gc(_gl);
        gc.appendRow(_lblInput) << _editInput << _btnInput;
        gc.appendRow(_lblOutput) << _editOutput << _btnOutput;
        gc.appendRow(_lblConfig) << _editConfig << _btnConfig;
        gc.appendRow(_lblGenerator) << _neGenerator;
        gc.appendRow(_lblStatus) << _editStatus;
        gc.appendRow(_lblProgress) << _piProgress << _editProgress;
        gc.appendRow(_btnInspect) << _btnConvert;
        setLayout(&_gl);
        handleUserActions();
    }
    
    ~SimplePluginView()
    {
        if (_progress.running)//sprijecava duplo pokretanje konverzije
            _progress.running = false;
        joinFinishedThreads();
    }
    
    td::String getOutFileName() const
    {
        return _editOutput.getText();
    }
};

class MinimalPluginWindow : public gui::Window
{
protected:
    SimplePluginView _view;
    
    void onClose() override final
    {
        onClosedPluginWindow();
    }
public:
    MinimalPluginWindow(sc::IPlugin* pIPlugin,
                        const sc::IPlugin::CallBack& onComplete)
    : gui::Window(gui::Size(800, 600))
    , _view(pIPlugin, onComplete)
    {
        setTitle("Turbine Governor Type I");
        setCentralView(&_view);
    }
    
    td::String getOutFileName() const
    {
        return _view.getOutFileName();
    }
};

class Plugin : public sc::IPlugin
{
    MemoryArchiveContainer _outArchives;
    gui::Window* _pWnd = nullptr;
    td::String _outFileName;

    void log(const char* message) const
    {
        tgtiTrace(message);
    }
public:
    Plugin()
    {
        for (size_t i = 0; i < size_t(ArchType::NA); ++i)
            _outArchives[i] = nullptr;
        _outFileName = "";
        log("Plugin constructed");
    }
    
    void show(gui::Window* parentWnd,
              MemoryArchiveContainer& archives,
              td::UINT4 wndID,
              const sc::IPlugin::Cleaner& cleaner,
              const sc::IPlugin::CallBack& onComplete) override final
    {
        log("show entered");
        for (size_t i = 0; i < size_t(ArchType::NA); ++i)
            _outArchives[i] = archives[i];
        
        (void) onComplete;
        (void) parentWnd;
        (void) wndID;
        (void) cleaner;
        
        if (_pWnd)
            _pWnd->setFocus();
        else
        {
            log("before standalone minimal window allocation");
            _pWnd = new MinimalPluginWindow(this, onComplete);
            log("after standalone minimal window allocation");
            log("before standalone minimal window open");
            _pWnd->open();
        }
        log("standalone minimal window opened");
    }
    
    td::String getMenuName() const override final
    {
        log("getMenuName");
        return "Turbine Governor Type I";
    }
    
    arch::MemoryOut* getArchive(sc::IPlugin::ArchType type) override final
    {
        auto iType = size_t(type);
        if (iType >= getMaxSupportedArchiveParts())
            return nullptr;
        return _outArchives[iType];
    }
    
    MemoryArchiveContainer& getArchives() override final
    {
        return _outArchives;
    }
    
    td::String getOutFileName() const override final
    {
        if (_pWnd)
        {
            auto pTGTIWnd = dynamic_cast<MinimalPluginWindow*>(_pWnd);
            if (pTGTIWnd)
                return pTGTIWnd->getOutFileName();
        }
        return _outFileName;
    }
    
    size_t getMaxSupportedArchiveParts() const override final
    {
        return size_t(ArchType::NA);
    }
    
    ModelType getModelType() const override final
    {
        return ModelType::DAE;
    }
    
    void onClosedPluginWindow()
    {
        _pWnd = nullptr;
        log("window closed");
    }
};

static Plugin s_plugin;

void onClosedPluginWindow()
{
    s_plugin.onClosedPluginWindow();
}

extern "C"
{
TGTI_PLUGIN_API sc::IPlugin* getPluginInterface()
{
    tgtiTrace("getPluginInterface");
    return &s_plugin;
}
}

void ConversionProgress::setMessage(const td::String& message)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _message = message;
}

td::String ConversionProgress::getMessage()
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _message;
}

void ConversionProgress::reset()
{
    percent = 0;
    running = false;
    completed = false;
    failed = false;
    setMessage("Idle");
}
