#pragma once
#include <td/Types.h>

class SimulationParams
{
public:
    double pOrder = 0.70; //referenca snage turbine, zadaje dispecer
    double omegaRef = 1.00; //referentna sinhrona brzina motora
    double omegaStep = -0.01; //korak pada brzine nakon poremecaja
    double nominalFrequency = 50.0; //nazivna frekvencija mreze
    double gridFrequency = 49.5; //frekvencija mreze nakon poremecaja
    double stepTime = 1.00; //trenutak u kojem nastaje poremecaj

    double r = 0.02; //droop, manji da dobijemo agresivniji odziv
    double ts = 0.10; //konstanta servo motora
    double tc = 0.45; //konstanta parne komore
    double t3 = 0.00; //vremenska konstanta turbine u brojniku
    double t4 = 0.01; //vremenska kontanta turbine u nazivniku
    double t5 = 7.00; //obicno izmedju 3 i 10, kasnjenje reheatera
    double pMax = 1.20;//maksimalno ogranicenje otvora ventila
    double pMin = 0.30; //minimalno ogranicenje otvora ventila

    double dTime = 0.005; //korak integracije
    double endTime = 30.0; //trajanje simulacije

    td::ColorID mechanicalPowerColor = td::ColorID::DodgerBlue;
    td::ColorID speedColor = td::ColorID::Red;
    td::ColorID servoColor = td::ColorID::DarkGreen;
};
