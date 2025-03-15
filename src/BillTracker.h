#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <algorithm>
#include <wx/wx.h>
#include <wx/panel.h>
#include <wx/datectrl.h>
#include <wx/valnum.h>
#include <wx/listctrl.h>
#include "CommonTools.h"
#include <wx/simplebook.h>
#include <wx/statline.h>

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
    std::shared_ptr<Bill> GetBill(int ix);
    std::string GetDate() const { return mDate; }

private:
    std::string       mDate; // The year and month this collection represents ("YYYY_MM")
    std::vector<std::shared_ptr<Bill>> mBills;
};

class BillTracker {
public:
    BillTracker() {}
    BillTracker(std::string datapath) : mBillsDataPath(datapath) {}
    ~BillTracker() {}

    bool LoadBills();
    bool StoreBills();
    bool StoreBill(int ix);

    bool AddBillMonth(const BillMonth& bm);

    // Access bill months
    int GetBillMonthCount() { return int(mBillData.size()); }
    std::shared_ptr<BillMonth> GetBillMonth(int ix) { assert(ix >= 0 && ix < int(mBillData.size())); return mBillData[ix]; }
private:
    std::string ParseFileName(std::string filename);
private:
    std::string mBillsDataPath = "";
    std::vector<std::shared_ptr<BillMonth>> mBillData;

};

class BillPanel : public wxPanel {
public:
    BillPanel(wxWindow* parent, BillTracker* bt);
private:
    class BillPanelRow {
    public:
        BillPanelRow(wxWindow* parent, std::shared_ptr<Bill> bill) : mParent(parent), mBill(bill) {}
        wxBoxSizer* GetLayout();
    private:
        wxWindow* mParent;
        std::shared_ptr<Bill> mBill;

        wxBoxSizer* mRowSizer;
        wxStaticText* mCreditorLabel;
        wxTextCtrl* mBillAmountCtrl;
        wxCheckBox* mHasPaid;
        wxDatePickerCtrl* mPaidDate;
    };

    class BillCollectionPanel : public wxPanel {
    public:
        BillCollectionPanel(wxWindow* parent, std::shared_ptr<BillMonth> bm);
    private:
        wxWindow* mParent;
        std::shared_ptr<BillMonth> mBillCollection;

        std::vector<std::shared_ptr<BillPanelRow>> mRows;
        wxBoxSizer* mSizer;
    };

    BillTracker* mBillTracker;
    wxBoxSizer* mTopSizer;
};