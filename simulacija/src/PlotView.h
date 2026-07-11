#pragma once
#include "SimulationParams.h"
#include "TurbineGovernorModel.h"
#include <gui/Canvas.h>
#include <gui/DrawableString.h>
#include <gui/Shape.h>
#include <cmath>
#include <cstdio>
#include <vector>

class PlotView : public gui::Canvas
{
protected:
    const std::vector<SimulationPoint>* _pPoints = nullptr;
    SimulationParams _params;
    size_t _visiblePoints = 0;
    gui::DrawableString _title;
    gui::DrawableString _legendTau;
    gui::DrawableString _legendOmega;
    gui::DrawableString _legendServo;

    static double ensureAtLeast(double value, double minValue)
    {
        if (value < minValue)
            return minValue;
        return value;
    }

    static size_t smallerSize(size_t first, size_t second)
    {
        if (first < second)
            return first;
        return second;
    }

    static double normalized(double value, double minValue, double maxValue)//min i max skalira na 0 do 1 za pozicioniranje ventila i parne struje u animaciji
    {
        double span = ensureAtLeast(maxValue - minValue, 1e-9);
        double n = (value - minValue) / span;
        if (n < 0.0)
            return 0.0;
        if (n > 1.0)
            return 1.0;
        return n;
    }

    SimulationPoint currentPoint() const
    {
        SimulationPoint point;
        if (!_pPoints || _pPoints->empty())
            return point;
        size_t pos = _visiblePoints;
        if (pos == 0)
            pos = 1;
        pos = smallerSize(pos, _pPoints->size());
        return (*_pPoints)[pos - 1];
    }

    gui::Point mapPoint(const gui::Rect& plotRect, double t, double y, double yMin, double yMax) const //pretvara (t,y) u pikselske koordinate unutar pravougaonika grafa
    {
        double xNorm = t / ensureAtLeast(_params.endTime, 1e-9);
        double yNorm = (y - yMin) / ensureAtLeast(yMax - yMin, 1e-9);
        gui::Point p;
        p.x = plotRect.left + xNorm * plotRect.width();
        p.y = plotRect.top + (1.0 - yNorm) * plotRect.height();
        return p;
    }

    void drawCurve(const gui::Rect& plotRect, double yMin, double yMax, td::ColorID color, int series) const
        //crta jednu krivulju,getValue bira koja se varijabla crta, po series parametru i ide kroz _visiblePoints tacaka i crta linije izmedju
    {
        if (!_pPoints || _pPoints->size() < 2 || _visiblePoints < 2)
            return;

        size_t n = smallerSize(_visiblePoints, _pPoints->size());
        auto getValue = [series](const SimulationPoint& p) {
            if (series == 0)
                return p.tauM;
            if (series == 1)
                return p.omega;
            return p.pServo;
        };

        gui::Point prev = mapPoint(plotRect, (*_pPoints)[0].t, getValue((*_pPoints)[0]), yMin, yMax);
        for (size_t i = 1; i < n; ++i)
        {
            gui::Point curr = mapPoint(plotRect, (*_pPoints)[i].t, getValue((*_pPoints)[i]), yMin, yMax);
            gui::Shape::drawLine(prev, curr, color, 2);
            prev = curr;
        }
    }

    void drawOval(const gui::Rect& r, td::ColorID fill, td::ColorID line, float lineWidth) const
    {
        gui::Shape shape;
        shape.createOval(r, lineWidth);
        shape.drawFillAndWire(fill, line, lineWidth);
    }

    void drawSteamArrow(gui::Point from, gui::Point to, float width, td::ColorID color) const
    {
        gui::Shape::drawLine(from, to, color, width, td::LinePattern::Solid, 0.85f);
        gui::Point h1(to.x - 16, to.y - 9);
        gui::Point h2(to.x - 16, to.y + 9);
        gui::Shape::drawLine(h1, to, color, width * 0.55f, td::LinePattern::Solid, 0.85f);
        gui::Shape::drawLine(h2, to, color, width * 0.55f, td::LinePattern::Solid, 0.85f);
    }

    void drawPlant(const gui::Rect& area)//crtanje postrojenja
    {
        SimulationPoint point = currentPoint();
        double valveOpen = normalized(point.pIn, _params.pMin, _params.pMax);//otvorenost ventila
        double steam = normalized(point.tauM, _params.pMin, _params.pMax);
        double rotorAngle = 0.0;
        if (_pPoints && !_pPoints->empty())
        {
            size_t n = smallerSize(_visiblePoints, _pPoints->size());
            for (size_t i = 1; i < n; ++i)//integrira ugaonu brzinu da dobije ugao rotora, pa se lopatice rotiraju ovisno o omega
            {
                double dt = (*_pPoints)[i].t - (*_pPoints)[i - 1].t;
                rotorAngle += (*_pPoints)[i].omega * dt * 42.0;//42 je estetski da rotacija bude vidljiva
            }
        }
        double pi = 3.14159265358979323846;

        gui::Shape::drawRect(area, 0.06f, td::ColorID::SysText);

        gui::Point pipeIn(area.left + 35, area.top + 95);
        gui::Point valveCenter(area.left + 190, area.top + 95);
        gui::Point turbineCenter(area.left + 410, area.top + 95);
        gui::Point generatorCenter(area.left + 640, area.top + 42);

        drawSteamArrow(pipeIn, {valveCenter.x - 45, valveCenter.y}, 7.0f + (float) (8.0 * steam), td::ColorID::LightSkyBlue);

        gui::Rect valveBody(valveCenter.x - 35, valveCenter.y - 45, valveCenter.x + 35, valveCenter.y + 45);
        gui::Shape::drawRect(valveBody, td::ColorID::LightGray, td::ColorID::SlateGray, 2);
        double gateY = valveBody.bottom - 12 - valveOpen * 62.0;
        gui::Shape::drawRect({valveBody.left + 9, gateY, valveBody.right - 9, gateY + 10}, td::ColorID::OrangeRed);
        gui::Shape::drawLine({valveCenter.x, valveBody.top - 28}, {valveCenter.x, gateY}, td::ColorID::SlateGray, 3);

        drawSteamArrow({valveCenter.x + 42, valveCenter.y}, {turbineCenter.x - 95, turbineCenter.y}, 6.0f + (float) (10.0 * steam), td::ColorID::DeepSkyBlue);

        gui::Rect turbineShell(turbineCenter.x - 90, turbineCenter.y - 58, turbineCenter.x + 90, turbineCenter.y + 58);
        drawOval(turbineShell, td::ColorID::SteelBlue, td::ColorID::SysText, 2);
        drawOval({turbineCenter.x - 42, turbineCenter.y - 42, turbineCenter.x + 42, turbineCenter.y + 42}, td::ColorID::LightSteelBlue, td::ColorID::Navy, 2);

        for (int i = 0; i < 6; ++i)
        {
            double a = rotorAngle + i * pi / 3.0;
            gui::Point bladeEnd(turbineCenter.x + std::cos(a) * 38.0, turbineCenter.y + std::sin(a) * 38.0);
            gui::Shape::drawLine(turbineCenter, bladeEnd, td::ColorID::Navy, 5);
        }
        drawOval({turbineCenter.x - 9, turbineCenter.y - 9, turbineCenter.x + 9, turbineCenter.y + 9}, td::ColorID::Gold, td::ColorID::DarkGoldenRod, 2);

        gui::Shape::drawLine({turbineCenter.x + 92, turbineCenter.y}, {generatorCenter.x - 55, generatorCenter.y}, td::ColorID::SlateGray, 6);
        gui::Rect gen(generatorCenter.x - 55, generatorCenter.y - 45, generatorCenter.x + 55, generatorCenter.y + 45);
        gui::Shape::drawRect(gen, td::ColorID::Gainsboro, td::ColorID::SysText, 2);
        drawOval({generatorCenter.x - 28, generatorCenter.y - 28, generatorCenter.x + 28, generatorCenter.y + 28}, td::ColorID::WhiteSmoke, td::ColorID::SlateGray, 2);

        drawSteamArrow({turbineCenter.x + 92, turbineCenter.y + 36}, {area.right - 35, turbineCenter.y + 36}, 4.0f + (float) (6.0 * steam), td::ColorID::LightCyan);

        char buffer[160];
        std::snprintf(buffer, sizeof(buffer), "valve %.0f%%   steam/tau_m %.3f pu   omega %.4f pu", valveOpen * 100.0, point.tauM, point.omega);
        gui::DrawableString values(buffer);
        values.draw({area.left + 30, area.bottom - 28}, gui::Font::ID::SystemNormal, td::ColorID::SysText);

        gui::DrawableString valveText("governor + servo valve");
        valveText.draw({valveCenter.x - 68, valveBody.bottom + 10}, gui::Font::ID::SystemSmaller, td::ColorID::SysText);
        gui::DrawableString turbineText("steam turbine");
        turbineText.draw({turbineCenter.x - 42, turbineShell.bottom + 10}, gui::Font::ID::SystemSmaller, td::ColorID::SysText);
        gui::DrawableString genText("generator/load");
        genText.draw({generatorCenter.x - 42, gen.bottom + 10}, gui::Font::ID::SystemSmaller, td::ColorID::SysText);
    }

    void drawPlot(const gui::Rect& plotRect, const gui::Size& sz)
    {
        gui::Point p1(plotRect.left, plotRect.top);
        gui::Point p2(plotRect.left, plotRect.bottom);
        gui::Shape::drawLine(p1, p2, td::ColorID::SysText, 1);
        p1 = {plotRect.left, plotRect.bottom};
        p2 = {plotRect.right, plotRect.bottom};
        gui::Shape::drawLine(p1, p2, td::ColorID::SysText, 1);

        double yMin = 0.25;
        double yMax = 1.25;
        drawCurve(plotRect, yMin, yMax, _params.mechanicalPowerColor, 0);
        drawCurve(plotRect, yMin, yMax, _params.speedColor, 1);
        drawCurve(plotRect, yMin, yMax, _params.servoColor, 2);

        _legendTau.draw({80, sz.height - 42}, gui::Font::ID::SystemNormal, _params.mechanicalPowerColor);
        _legendOmega.draw({180, sz.height - 42}, gui::Font::ID::SystemNormal, _params.speedColor);
        _legendServo.draw({280, sz.height - 42}, gui::Font::ID::SystemNormal, _params.servoColor);

        char buffer[128];
        std::snprintf(buffer, sizeof(buffer), "t [s]  |  y [pu]  |  grid frequency %.2f Hz after %.2f s", _params.gridFrequency, _params.stepTime);
        gui::DrawableString label(buffer);
        label.draw({70, sz.height - 72}, gui::Font::ID::SystemNormal, td::ColorID::SysText);
    }

public:
    PlotView()
    : _title("Turbine Governor Type I - steam valve and step response")
    , _legendTau("tau_m")
    , _legendOmega("omega")
    , _legendServo("servo")
    {
    }

    void setData(const std::vector<SimulationPoint>* pPoints, const SimulationParams& params)
    {
        _pPoints = pPoints;
        _params = params;
        _visiblePoints = pPoints ? pPoints->size() : 0;
        reDraw();
    }

    void setVisiblePoints(size_t visiblePoints)
    {
        _visiblePoints = visiblePoints;
        reDraw();
    }

    size_t getNoOfPoints() const
    {
        if (!_pPoints)
            return 0;
        return _pPoints->size();
    }

    void onDraw(const gui::Rect& rect) override //poziva se svaki put kada treba nacrtati view, dijeli ekran na gornji (animacija) i donji (grafovi)
    {
        gui::Size sz;
        getSize(sz);

        _title.draw({70, 18}, gui::Font::ID::SystemLarger, td::ColorID::SysText);

        gui::CoordType plantBottom = 250;
        if (sz.height > 760)
            plantBottom = 300;
        gui::Rect plantRect(55, 52, sz.width - 35, plantBottom);
        drawPlant(plantRect);

        gui::Rect plotRect(70, plantBottom + 55, sz.width - 35, sz.height - 95);
        drawPlot(plotRect, sz);
    }
};



/*POTENCIJALNE POPRAVKE:
* brojevi na x i y osama
* beskonacno trajanje simulacije

*/