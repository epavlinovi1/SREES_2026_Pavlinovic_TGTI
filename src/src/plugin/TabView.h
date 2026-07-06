#pragma once

#include <gui/StandardTabView.h>
#include "ViewConverter.h"
#include "ViewOptions.h"

class TabView : public gui::StandardTabView
{
protected:
    ViewConverter _converter;
    ViewOptions _options;
public:
    TabView(sc::IPlugin* pIPlugin, const sc::IPlugin::CallBack& onComplete)
    : _converter(pIPlugin, onComplete)
    {
        _converter.setOptions(&_options);
        addView(&_converter, "Converter");
        addView(&_options, "Options");
    }
    
    td::String getOutFileName() const
    {
        return _converter.getOutFileName();
    }
};
