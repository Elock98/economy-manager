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
    mDataDir = home + "/EconoManager";
    if (!FileSystem::IsDirectory((mDataDir).c_str()))
        FileSystem::MakeDir((mDataDir).c_str());

    mBillsDir = mDataDir + "/Bills";
    if (!FileSystem::IsDirectory((mBillsDir).c_str()))
        FileSystem::MakeDir((mBillsDir).c_str());
    return true;
}

MainFrame::MainFrame(const wxString& title) : wxFrame(NULL, wxID_ANY, title) {

    // Create a bill tracker
    BillTracker* bt = new BillTracker(wxGetApp().GetBillsDir());

    // Load bills will create one manager for this month if it does not exist
    bt->LoadBills();

    if (bt->GetBillMonthCount() <= 0)
        return;

    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);
    BillPanel* billpanel = new BillPanel(this, bt);

    topsizer->Add(billpanel, 1, wxEXPAND | wxLEFT | wxTOP | wxBOTTOM);

    SetSizer(topsizer);
    Maximize(true);
    Layout();

}

void MainFrame::OnExit(wxCommandEvent& evt) {
    Close(true);
}