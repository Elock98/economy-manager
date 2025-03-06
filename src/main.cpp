#include "main.h"

wxIMPLEMENT_APP(EconoManagerApp);

bool EconoManagerApp::OnInit() {
    MainFrame *frame = new MainFrame("EconoManager");
    frame->Show(true);
    return true;
}

MainFrame::MainFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title) {

}