#ifndef MYAPP_H
#define MYAPP_H

#include <wx/wx.h>
#include <cstdlib>
#include "CommonTools.h"
#include "BillTracker.h"

class EconoManagerApp : public wxApp {
public:
    virtual bool OnInit();
    bool SetupDataDir();
};

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);
};

#endif // MYAPP_H
