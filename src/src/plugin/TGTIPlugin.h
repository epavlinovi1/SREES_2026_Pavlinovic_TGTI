#pragma once

#include <compiler/Definitions.h>//za detekciju kompajlera/platforme
#include <sc/IPlugin.h>//za dtwin
#include <td/String.h>//string ali iz natID
#include <atomic>//?????????????
#include <mutex>//???????????????

#ifdef MU_WINDOWS//????????????
    #ifdef PLUGIN_EXPORTS
        #define TGTI_PLUGIN_API __declspec(dllexport)
    #else
        #define TGTI_PLUGIN_API __declspec(dllimport)
    #endif
#else
    #ifdef PLUGIN_EXPORTS
        #define TGTI_PLUGIN_API __attribute__((visibility("default")))
    #else
        #define TGTI_PLUGIN_API
    #endif
#endif

struct TGTIParameters
{
    double R = 0.02;
    double Ts = 0.10;
    double Tc = 0.45;
    double T3 = 0.0;
    double T4 = 0.01;
    double T5 = 50.0;
    double pmax = 1.20;
    double pmin = 0.30;
};

struct TGTIOptions
{
    td::String modelName;
    td::String configFileName;//putanja do xml fajla
    td::String selectedCaseName;
    td::INT4 generatorNo = 1;//koji red u matrici slucaja dobija tgti dinamiku??????????????????PROVJERITI
    td::INT4 maxIter = 20;
    float dTime = 0.005f;
    float endTime = 10.0f;
    bool useStandardForOthers = true;
    TGTIParameters params;
};

//IMA VEZE SA ATOMIC I MUTEX SA POCETKA?????????????????????
class ConversionProgress
{
    std::mutex _mutex;
    td::String _message;
public:
    std::atomic<int> percent = 0;
    std::atomic<bool> running = false;
    std::atomic<bool> completed = false;
    std::atomic<bool> failed = false;
    
    void setMessage(const td::String& message);
    td::String getMessage();
    void reset();
};

void onClosedPluginWindow();//poziva se kada zatvorimo plugin prozor

bool createTGTIModel(const td::String& inputFileName,//putanja do MATPOWER
                     const td::String& outFileName,//putanja do dmodl
                     sc::IPlugin* pIPlugin,
                     const TGTIOptions& options,
                     ConversionProgress& progress,
                     td::String& status);//varijable za guiu

