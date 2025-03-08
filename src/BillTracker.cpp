#include "BillTracker.h"

//-----------------------------------------
//  BillMonth
//-----------------------------------------

int BillMonth::BillCount() {
    return static_cast<int>(mBills.size());
}

void BillMonth::AddBill(Bill bill) {
    mBills.push_back(bill);
}

void BillMonth::RemoveBill(int ix) {
    if (ix < 0 || ix >= static_cast<int>(mBills.size())) {
        assert(false);
        return;
    }
    mBills.erase(mBills.begin() + ix);
}

Bill& BillMonth::GetBill(int ix) {
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
        mBillData.push_back(bm);
    }

    // Sort the data
    sort(mBillData.begin(), mBillData.end(), [](const BillMonth& a, const BillMonth& b) {
        return a.GetDate() > b.GetDate();
    });

    // Check if current year and month exist in our data (should be the first in the vector)
    std::string currentDateTime = Time::TimeToStr(Time::GetCurrentTime(), "%Y_%m");
    if (int(mBillData.size()) == 0 || mBillData[0].GetDate() != currentDateTime)
        mBillData.insert(mBillData.begin(), BillMonth(currentDateTime));

    return true;
}

bool BillTracker::StoreBills() {
    const std::string filenamePrefix = mBillsDataPath + "/Bills_";

    for (auto billMonth : mBillData) {
        std::string filename = filenamePrefix + billMonth.GetDate() + ".csv";
        std::string data = "";
        for (int ix = 0; ix < billMonth.BillCount(); ix++) {
            auto bill = billMonth.GetBill(ix);
            data += String::JoinStrings({bill.mCreditor, bill.mBillAmount, bill.mIsPaid, bill.mPaidDate}, ",");
            if (ix != billMonth.BillCount() - 1) // Append newline for all but the last
                data += "\n";
        }
        FileSystem::WriteFile(filename, data);
    }
    return true;
}

bool BillTracker::AddBillMonth(const BillMonth& bm) {
    if (std::any_of(mBillData.begin(), mBillData.end(), [&bm](BillMonth& lhs) {
        return lhs.GetDate() == bm.GetDate();
    }))
        return false;

    mBillData.emplace_back(bm);
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
