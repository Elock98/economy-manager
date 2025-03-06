#ifndef MYAPP_H
#define MYAPP_H

#include <wx/wx.h>

class EconoManagerApp : public wxApp {
public:
    virtual bool OnInit();
};

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);
};

#endif // MYAPP_H
