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
    const std::string filenamePrefix = mBillsDataPath + "/Bills_";

    for (auto billMonth : mBillData) {
        std::string filename = filenamePrefix + billMonth->GetDate() + ".csv";
        std::string data = "";
        for (int ix = 0; ix < billMonth->BillCount(); ix++) {
            auto bill = billMonth->GetBill(ix);
            data += String::JoinStrings({bill->mCreditor, bill->mBillAmount, bill->mIsPaid, bill->mPaidDate}, ",");
            if (ix != billMonth->BillCount() - 1) // Append newline for all but the last
                data += "\n";
        }
        FileSystem::WriteFile(filename, data);
    }
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
    : wxPanel(parent, wxID_ANY)
{
    mTopSizer = new wxBoxSizer(wxHORIZONTAL);

    // ListCtrl will control switching between months
    wxListCtrl* monthSelector = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_NO_HEADER);
    monthSelector->SetMinSize(wxSize(200, -1));
    monthSelector->InsertColumn(0, "");

    // Collect the month panels in a tab-less notebook
    wxSimplebook* monthBook = new wxSimplebook(this, wxID_ANY);

    for (int i = 0; i < bt->GetBillMonthCount(); ++i) {
        auto bm = bt->GetBillMonth(i);
        monthSelector->InsertItem(i, bm->GetDate());
        monthBook->AddPage(new BillCollectionPanel(monthBook, bm), "");
    }

    mTopSizer->Add(monthSelector, 0, wxEXPAND | wxLEFT | wxTOP | wxBOTTOM, 5);
    mTopSizer->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(5, -1), wxLI_VERTICAL), 0, wxEXPAND | wxALL, 5);
    mTopSizer->Add(monthBook, 1, wxEXPAND | wxALL, 20);

    monthSelector->Bind(wxEVT_LIST_ITEM_SELECTED, [monthBook](wxListEvent& event) {
        monthBook->SetSelection(event.GetIndex());
    });

    SetSizer(mTopSizer);
    Layout();

}

wxBoxSizer* BillPanel::BillPanelRow::GetLayout() {
    mRowSizer = new wxBoxSizer(wxHORIZONTAL);

    mCreditorLabel = new wxStaticText(mParent, wxID_ANY, mBill->mCreditor);

    wxFloatingPointValidator<double> numValidator(2);
    numValidator.SetMin(0.0);
    numValidator.SetMax(99999.99);

    mBillAmountCtrl = new wxTextCtrl(mParent, wxID_ANY, mBill->mBillAmount, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT, numValidator);

    mHasPaid = new wxCheckBox(mParent, wxID_ANY, "");
    mHasPaid->SetValue(mBill->mIsPaid == "true");

    mPaidDate = new wxDatePickerCtrl(mParent, wxID_ANY, wxDefaultDateTime, wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);

    mRowSizer->Add(mCreditorLabel, 2, wxEXPAND | wxALL, 5);
    mRowSizer->Add(mBillAmountCtrl, 1, wxEXPAND | wxALL, 5);
    mRowSizer->Add(mPaidDate, 1, wxEXPAND | wxALL, 5);
    mRowSizer->Add(mHasPaid, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);

    return mRowSizer;

}

BillPanel::BillCollectionPanel::BillCollectionPanel(wxWindow* parent, std::shared_ptr<BillMonth> bm)
    : wxPanel(parent, wxID_ANY), mParent(parent), mBillCollection(bm)
{
    mSizer = new wxBoxSizer(wxVERTICAL);

    // Get a BillPanelRow for each bill
    for (int ix = 0; ix < mBillCollection->BillCount(); ++ix) {
        BillPanelRow* bpr = new BillPanelRow(this, mBillCollection->GetBill(ix));
        mRows.emplace_back(bpr);
        mSizer->Add(bpr->GetLayout(), 0, wxEXPAND);
    }
    SetSizer(mSizer);
    Layout();
}
