#pragma once
#include "SimulationParams.h"
#include <dense/Matrix.h>
#include <mem/Buffer.h>
#include <sparse/ISolver.h>
#include <cmath>
#include <vector>
//koja je razlika izmedju struct i class
struct SimulationPoint//
{
    double t = 0; //trenutno vrijeme
    double omega = 1; //ugaona brzina u pu
    double pIn = 0; //ulaz u turbinu nakon limitera
    double pServo = 0; //izlaz servo ventila
    double tauM = 0; //mehanicki momenat, izlaz reheatera
};

class TurbineGovernorModel
{
protected:
    SimulationParams _params;
    dense::Matrix<double> _state;//vektor stanja
    std::vector<SimulationPoint> _points;//tacke simulacije

    static double limit(double value, double minValue, double maxValue)
    {
        if (value < minValue)
            return minValue;
        if (value > maxValue)
            return maxValue;
        return value;
    }

    static double ensureAtLeast(double value, double minValue)
    {
        if (value < minValue)
            return minValue;
        return value;
    }

    double omegaAt(double t) const//nagli pad frekvencije
    {
        if (t < _params.stepTime)
            return _params.omegaRef;
        return _params.omegaRef + _params.omegaStep;
    }

    void stateDerivative(double t, const dense::Matrix<double>& x, dense::Matrix<double>& dx) //diferencijalne jednacine tj izvodi stanja
    {
        auto xr = x.getManipulator();//
        auto dxw = dx.getManipulator();//

        double governor = xr(0, 0);
        double servo = xr(1, 0);
        double leadLagState = xr(2, 0);//
        double reheater = xr(3, 0);

        double omega = omegaAt(t);
        double speedSignal = _params.pOrder + (_params.omegaRef - omega) / _params.r;//ulaz u governor, ako omega padne, signal raste i traži više snage tj Pin u knjizi prije limitera
        double pIn = limit(governor, _params.pMin, _params.pMax);
        double transientGain = servo;
        //ima veze sa lead-lag filterom
        if (_params.t4 > 1e-6)//
        {
            double ratio = _params.t3 / _params.t4;
            transientGain = ratio * servo + (1.0 - ratio) * leadLagState;
        }
        //svaki red jedna diferencijalna jednacina iz knjige
        dxw(0, 0) = (speedSignal - governor) / _params.ts;
        dxw(1, 0) = (pIn - servo) / _params.tc;
        dxw(2, 0) = (servo - leadLagState) / _params.t4;
        dxw(3, 0) = (transientGain - reheater) / _params.t5;
    }

    void rk4Step(double t, double h)//metoda integracije koja koristi 4 procjene izvoda po koraku
    {
        dense::Matrix<double> k1(4, 1, nullptr, true);
        dense::Matrix<double> k2(4, 1, nullptr, true);
        dense::Matrix<double> k3(4, 1, nullptr, true);
        dense::Matrix<double> k4(4, 1, nullptr, true);
        dense::Matrix<double> tmp(4, 1, nullptr, true);

        stateDerivative(t, _state, k1);
        auto x = _state.getManipulator();
        auto tw = tmp.getManipulator();
        auto k1r = k1.getManipulator();
        for (td::UINT4 i = 0; i < 4; ++i)
            tw(i, 0) = x(i, 0) + 0.5 * h * k1r(i, 0);
//????????????????????????
        stateDerivative(t + 0.5 * h, tmp, k2);
        auto k2r = k2.getManipulator();
        for (td::UINT4 i = 0; i < 4; ++i)
            tw(i, 0) = x(i, 0) + 0.5 * h * k2r(i, 0);

        stateDerivative(t + 0.5 * h, tmp, k3);
        auto k3r = k3.getManipulator();
        for (td::UINT4 i = 0; i < 4; ++i)
            tw(i, 0) = x(i, 0) + h * k3r(i, 0);

        stateDerivative(t + h, tmp, k4);
        auto k4r = k4.getManipulator();
        for (td::UINT4 i = 0; i < 4; ++i)
            x(i, 0) += h * (k1r(i, 0) + 2.0 * k2r(i, 0) + 2.0 * k3r(i, 0) + k4r(i, 0)) / 6.0;
    }

    void touchSparseMatrixApi()//demonstracija sparse matrica
    {
        sparse::DblSolverReleaser solver(sparse::createDblSolver(4, 4, sparse::Symmetry::NonSymmetric, sparse::SolverType::LU));
        if (!solver.ptr())
            return;
        auto& s = *(solver.ptr());
        s.addTriple(0, 0, 1.0);
        s.addTriple(1, 1, 1.0);
        s.addTriple(2, 2, 1.0);
        s.addTriple(3, 3, 1.0);
    }

public:
    TurbineGovernorModel()
    : _state(4, 1, nullptr, true)
    {
    }

    void setParams(const SimulationParams& params)
    {
        _params = params;
        _params.ts = ensureAtLeast(_params.ts, 1e-6);
        _params.tc = ensureAtLeast(_params.tc, 1e-6);
        _params.t4 = ensureAtLeast(_params.t4, 1e-6);
        _params.t5 = ensureAtLeast(_params.t5, 1e-6);
        _params.r = ensureAtLeast(_params.r, 1e-6);
        _params.dTime = ensureAtLeast(_params.dTime, 1e-5);
        _params.endTime = ensureAtLeast(_params.endTime, _params.dTime);
    }

    const SimulationParams& getParams() const
    {
        return _params;
    }

    const std::vector<SimulationPoint>& getPoints() const
    {
        return _points;
    }

    void solve()//pokretanje simulacije
    {
        touchSparseMatrixApi();
        _points.clear();

        auto x = _state.getManipulator();
        //incijalizacija, sistem u ravnotezi
        x(0, 0) = _params.pOrder;
        x(1, 0) = limit(_params.pOrder, _params.pMin, _params.pMax);
        x(2, 0) = x(1, 0);
        x(3, 0) = x(2, 0);

        for (double t = 0.0; t <= _params.endTime + 1e-9; t += _params.dTime)
        {
            //spremi trenutnu tacku u _points
            auto xr = _state.getManipulator();
            SimulationPoint point;
            point.t = t;
            point.omega = omegaAt(t);
            point.pIn = limit(xr(0, 0), _params.pMin, _params.pMax);
            point.pServo = xr(1, 0);
            point.tauM = xr(3, 0);
            _points.push_back(point);
            rk4Step(t, _params.dTime);//izracunaj sljedece stanje
        }
    }
};
