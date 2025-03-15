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

    std::string GetDataDir() { return mDataDir; }
    std::string GetBillsDir() { return mBillsDir; }
private:
    std::string mDataDir = "";
    std::string mBillsDir = "";
};

class MainFrame : public wxFrame {
public:
    MainFrame(const wxString& title);
    void OnExit(wxCommandEvent& evt);
};

#endif // MYAPP_H
