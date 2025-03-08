#pragma once
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include "CommonTools.h"

class Bill {
public:
    Bill();
    Bill(std::string creditor, std::string amount, std::string isPaid = "false", std::string paiddate = "")
        : mCreditor(creditor), mBillAmount(amount), mIsPaid(isPaid), mPaidDate(paiddate) { }
    std::string mCreditor;
    std::string      mBillAmount;
    std::string        mIsPaid;
    std::string mPaidDate;
};

// Collection of bills for a specific year and month
class BillMonth {
public:
    BillMonth(std::string date) : mDate(date) {}
    int BillCount();
    void AddBill(Bill bill);
    void RemoveBill(int ix);
    Bill& GetBill(int ix);
    std::string GetDate() const { return mDate; }

private:
    std::string       mDate; // The year and month this collection represents ("YYYY_MM")
    std::vector<Bill> mBills;
};

class BillTracker {
public:
    BillTracker() {}
    BillTracker(std::string datapath) : mBillsDataPath(datapath) {}
    ~BillTracker() {}

    bool LoadBills();
    bool StoreBills();

    bool AddBillMonth(const BillMonth& bm);

    // Access bill months
    int GetBillMonthCount() { return int(mBillData.size()); }
    BillMonth& GetBillMonth(int ix) { assert(ix >= 0 && ix < int(mBillData.size())); return mBillData[ix]; }
private:
    std::string ParseFileName(std::string filename);
private:
    std::string mBillsDataPath = "";
    std::vector<BillMonth> mBillData;

};