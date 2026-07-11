#pragma once
#include <gui/Image.h>
#include <gui/ToolBar.h>

class ToolBar : public gui::ToolBar
{
public:
    ToolBar(gui::Image* imgRun, gui::Image* imgReset)
    : gui::ToolBar("mainTB", 2)//pravimo toolbar sa imenom mainTB i 2 dugmeta
    {
        addItem(tr("start"), imgRun, tr("startTT"), 20, 0, 0, 10);//dodajemo dugmad(label,tooltip,ID koji odgovara onom u onActionItem)
        addItem(tr("reset"), imgReset, tr("resetTT"), 20, 0, 0, 20);
    }
};
