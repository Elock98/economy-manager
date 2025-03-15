#include "BillTracker.h"

//-----------------------------------------
//  BillMonth
//-----------------------------------------

int BillMonth::BillCount() {
    return static_cast<int>(mBills.size());
}

void BillMonth::AddBill(Bill bill) {
    mBills.push_back(std::make_shared<Bill>(bill));
}

void BillMonth::RemoveBill(int ix) {
    if (ix < 0 || ix >= static_cast<int>(mBills.size())) {
        assert(false);
        return;
    }
    mBills.erase(mBills.begin() + ix);
}

std::shared_ptr<Bill> BillMonth::GetBill(int ix) {
    assert(ix < BillCount());
    return mBills.at(ix);
}

//-----------------------------------------
//  BillTracker
//-----------------------------------------

bool BillTracker::LoadBills() {
    assert(FileSystem::IsDirectory(mBillsDataPath.c_str()));
    std::vector<std::string> files;

    assert(FileSystem::ListFiles(mBillsDataPath.c_str(), files));

    // Assume filenames being Bills_YYYY_MM.csv
    for (std::string file : files) {
        std::string date = ParseFileName(file);
        std::vector<std::array<std::string, 4>> fileData;
        if (!FileSystem::ReadStaticRowCSV(mBillsDataPath + "/" + file, fileData))
            return false;
        BillMonth bm(date);
        // Go through and create each bill
        for (auto bill : fileData) {
            Bill bl(bill[0], bill[1], bill[2], bill[3]);
            bm.AddBill(bl);
        }
        mBillData.push_back(std::make_shared<BillMonth>(bm));
    }

    // Sort the data
    sort(mBillData.begin(), mBillData.end(), [](const std::shared_ptr<BillMonth>& a, const std::shared_ptr<BillMonth>& b) {
        return a->GetDate() > b->GetDate();
    });

    // Check if current year and month exist in our data (should be the first in the vector)
    std::string currentDateTime = Time::TimeToStr(Time::GetCurrentTime(), "%Y_%m");
    if (int(mBillData.size()) == 0 || mBillData[0]->GetDate() != currentDateTime)
        mBillData.insert(mBillData.begin(), std::make_shared<BillMonth>(BillMonth(currentDateTime)));

    return true;
}

bool BillTracker::StoreBills() {
    bool rc = true;
    for (int ix = 0; ix < GetBillMonthCount(); ++ix) {
        rc = StoreBill(ix);
        if (!rc)
            break; // Stop if we failed
    }
    return rc;
}

bool BillTracker::StoreBill(int ix) {
    if (ix < 0 || ix > static_cast<int>(mBillData.size()))
        return false;

    const std::string filenamePrefix = mBillsDataPath + "/Bills_";

    auto billMonth = GetBillMonth(ix);
    std::string filename = filenamePrefix + billMonth->GetDate() + ".csv";
    std::string data = "";
    for (int ix = 0; ix < billMonth->BillCount(); ix++) {
        auto bill = billMonth->GetBill(ix);
        data += String::JoinStrings({bill->mCreditor, bill->mBillAmount, bill->mIsPaid, bill->mPaidDate}, ",");
        if (ix != billMonth->BillCount() - 1) // Append newline for all but the last
            data += "\n";
    }
    FileSystem::WriteFile(filename, data);
    return true;
}

bool BillTracker::AddBillMonth(const BillMonth& bm) {
    if (std::any_of(mBillData.begin(), mBillData.end(), [&bm](std::shared_ptr<BillMonth> lhs) {
        return lhs->GetDate() == bm.GetDate();
    }))
        return false;

    mBillData.emplace_back(std::make_shared<BillMonth>(bm));
    return true;
}

std::string BillTracker::ParseFileName(std::string filename) {
    // Input should be formatted as "Bills_YYYY_MM.csv"
    size_t pos = filename.find('_');
    if (pos == std::string::npos) {
        // No '_' found, this filename is corrupted
        return "";
    }
    return filename.substr(pos + 1, 7);
}

//-----------------------------------------
//  Bill UI
//-----------------------------------------

BillPanel::BillPanel(wxWindow* parent, BillTracker* bt)
    : wxPanel(parent, wxID_ANY), mBillTracker(bt)
{
    mTopSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* billCollectionSizer = new wxBoxSizer(wxVERTICAL);

    // ListCtrl will control switching between months
    mMonthSelector = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_NO_HEADER);
    mMonthSelector->SetMinSize(wxSize(200, -1));
    mMonthSelector->InsertColumn(0, "");

    // Collect the month panels in a tab-less notebook
    mMonthBook = new wxSimplebook(this, wxID_ANY);

    for (int i = 0; i < bt->GetBillMonthCount(); ++i) {
        auto bm = bt->GetBillMonth(i);
        mMonthSelector->InsertItem(i, bm->GetDate());
        mMonthBook->AddPage(new BillCollectionPanel(mMonthBook, bm), "");
    }

    wxButton* save = new wxButton(this, wxID_ANY, "Save");

    billCollectionSizer->Add(mMonthBook, 1, wxEXPAND | wxTOP | wxBOTTOM, 20);
    billCollectionSizer->Add(save, 0, wxALL, 5);


    mTopSizer->Add(mMonthSelector, 0, wxEXPAND | wxLEFT | wxTOP | wxBOTTOM, 5);
    mTopSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(5, -1), wxLI_VERTICAL), 0, wxEXPAND | wxALL, 5);
    mTopSizer->Add(billCollectionSizer, 1, wxEXPAND | wxTOP | wxBOTTOM, 20);

    mMonthSelector->Bind(wxEVT_LIST_ITEM_SELECTED, &BillPanel::OnSelection, this);

    save->Bind(wxEVT_BUTTON, &BillPanel::OnSaveButton, this);

    SetSizer(mTopSizer);
    Layout();

}

void BillPanel::OnSaveButton(wxCommandEvent& evt) {
    auto page = dynamic_cast<BillCollectionPanel*>(mMonthBook->GetCurrentPage());
    if (page == nullptr) {
        assert(false);
        return;
    }
    page->UpdateBillMonth();
    mBillTracker->StoreBill(mMonthBook->GetSelection());
}

void BillPanel::OnSelection(wxListEvent& evt) {
    mMonthBook->SetSelection(evt.GetIndex());
}

void BillPanel::BillPanelRow::Build() {
    mRowSizer = new wxBoxSizer(wxHORIZONTAL);

    mCreditorLabel = new wxStaticText(mParent, wxID_ANY, mBill->mCreditor);

    wxFloatingPointValidator<double> numValidator(2);
    numValidator.SetMin(0.0);
    numValidator.SetMax(99999.99);

    mBillAmountCtrl = new wxTextCtrl(mParent, wxID_ANY, mBill->mBillAmount, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, numValidator);

    mHasPaid = new wxCheckBox(mParent, wxID_ANY, "");
    mHasPaid->SetValue(mBill->mIsPaid == "true");

    mPaidDate = new wxDatePickerCtrl(mParent, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);

    wxBitmapButton* delButton = new wxBitmapButton(mParent, wxID_ANY, wxArtProvider::GetBitmap(wxART_CROSS_MARK, wxART_BUTTON));
    delButton->SetToolTip("Delete bill");

    mRowSizer->Add(mCreditorLabel, 2, wxEXPAND | wxALL, 5);
    mRowSizer->Add(mBillAmountCtrl, 1, wxEXPAND | wxALL, 5);
    mRowSizer->Add(mPaidDate, 1, wxEXPAND | wxALL, 5);
    mRowSizer->Add(mHasPaid, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
    mRowSizer->Add(new wxStaticLine(mParent, wxID_ANY, wxDefaultPosition, wxSize(5, -1), wxLI_VERTICAL), 0, wxEXPAND | wxALL, 5);
    mRowSizer->Add(delButton, 0, wxALL, 5);

    delButton->Bind(wxEVT_BUTTON, &BillPanelRow::OnDelete, this);
}

wxBoxSizer* BillPanel::BillPanelRow::GetLayout() {
    return mRowSizer;
}

void BillPanel::BillPanelRow::UpdateBillFromUI() {
    mBill->mBillAmount = mBillAmountCtrl->GetValue().ToStdString();
    mBill->mPaidDate = std::string(mPaidDate->GetValue().FormatISODate().mb_str());
    mBill->mIsPaid = mHasPaid->GetValue();
}

void BillPanel::BillPanelRow::OnDelete(wxCommandEvent& evt) {
    if (wxMessageBox("Are you sure you want to delete this bill?", "Confirm deletion", wxYES_NO | wxICON_WARNING) == wxNO)
        return;

    // Signal parent to remove me
    static_cast<BillCollectionPanel*>(mParent)->RemoveBill(mBill->mCreditor);
}

void BillPanel::BillCollectionPanel::UpdateBillMonth() {
    for (auto row : mRows) {
        row->UpdateBillFromUI();
    }
}

void BillPanel::BillCollectionPanel::RemoveBill(const std::string creditor) {
    // Remove bill with given creditor
    for (int ix = 0; ix < mBillCollection->BillCount(); ++ix) {
        auto bill = mBillCollection->GetBill(ix);
        if (bill->mCreditor == creditor) {
            mBillCollection->RemoveBill(ix);
            // Assume that the bill has the same ix in collection as row
            auto bpr = mRows.at(ix);
            mSizer->Detach(static_cast<wxSizer*>(bpr->GetLayout()));
            for (auto child : bpr->GetLayout()->GetChildren()) {
                if (child) child->GetWindow()->Destroy();
            }
            delete bpr->GetLayout();
            delete bpr;
            mRows.erase(mRows.begin() + ix);

            Layout();
            Refresh(true);
            mParent->Layout();
            mParent->Refresh(true);
            return;
        }
    }
}

BillPanel::BillCollectionPanel::BillCollectionPanel(wxWindow* parent, std::shared_ptr<BillMonth> bm)
    : wxPanel(parent, wxID_ANY), mParent(parent), mBillCollection(bm)
{
    mSizer = new wxBoxSizer(wxVERTICAL);

    if (mBillCollection->BillCount() > 0)
        mSizer->Add(new wxStaticLine(mParent, wxID_ANY, wxDefaultPosition, wxSize(5, -1), wxLI_HORIZONTAL), 0, wxEXPAND | wxALL, 5);
    // Get a BillPanelRow for each bill
    for (int ix = 0; ix < mBillCollection->BillCount(); ++ix) {
        BillPanelRow* bpr = new BillPanelRow(this, mBillCollection->GetBill(ix));
        mRows.emplace_back(bpr);
        bpr->Build();
        mSizer->Add(bpr->GetLayout(), 0, wxEXPAND);
        //mSizer->Add(new wxStaticLine(mParent, wxID_ANY, wxDefaultPosition, wxSize(5, -1), wxLI_HORIZONTAL), 0, wxEXPAND | wxALL, 5);
    }

    SetSizer(mSizer);
    Layout();
}
