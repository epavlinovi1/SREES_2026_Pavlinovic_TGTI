#pragma once

#include <gui/View.h>
#include <gui/TextEdit.h>
#include <gui/GridLayout.h>
#include <gui/GridComposer.h>

class ViewPreview : public gui::View
{
protected:
    gui::TextEdit _te;
    gui::GridLayout _gl;
public:
    ViewPreview()
    : _gl(1, 1)
    {
        _te.setAsReadOnly();
        _te.setText("Generated model preview will appear after Inspect or Convert.");
        gui::GridComposer gc(_gl);
        gc.appendRow(_te, 0);
        setLayout(&_gl);
    }
    
    void setPreview(const td::String& text)
    {
        _te.setText(text);
    }
};

