#include "main.h"

wxIMPLEMENT_APP(EconoManagerApp);

bool EconoManagerApp::OnInit() {

    if (!SetupDataDir())
        return false;

    MainFrame *frame = new MainFrame("EconoManager");
    frame->Show(true);
    return true;
}

bool EconoManagerApp::SetupDataDir() {
    std::string home = getenv("HOME");
    if (!FileSystem::IsDirectory((home + "/EconoManager").c_str()))
        FileSystem::MakeDir((home + "/EconoManager").c_str());

    if (!FileSystem::IsDirectory((home + "/EconoManager/Bills").c_str()))
        FileSystem::MakeDir((home + "/EconoManager/Bills").c_str());
    return true;
}

MainFrame::MainFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title) {
}